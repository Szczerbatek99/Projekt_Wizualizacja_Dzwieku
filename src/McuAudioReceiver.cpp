#include "McuAudioReceiver.h"
#include "config.h"
#include <QDebug>
#include <cstring>

/**
 * @brief Funkcja obliczająca standardowe CRC32 (takie samo jak w zlib / IEEE 802.3).
 */
static uint32_t calculateCRC32(const QByteArray &data) {
    uint32_t crc = 0xFFFFFFFF;
    for (int i = 0; i < data.size(); ++i) {
        crc ^= static_cast<uint8_t>(data[i]);
        for (int j = 0; j < 8; ++j) {
            crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
        }
    }
    return ~crc;
}

McuAudioReceiver::McuAudioReceiver(QObject *parent):
    QObject(parent), m_isActive(false), m_serialPort(new QSerialPort(this))
{
    // łączenie sygnału gotowości do odczytu ze slotem który odczytuje dane
    connect(m_serialPort, &QSerialPort::readyRead, this, &McuAudioReceiver::handleReadyRead);
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &McuAudioReceiver::handleError);
}

McuAudioReceiver::~McuAudioReceiver()
{
    stop();
}

bool McuAudioReceiver::start()
{
    if (m_isActive) return true;

    // konfiguracja portu
    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(baudRate); 
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serialPort->open(QIODevice::ReadOnly)) {
        m_isActive = true;
        m_buffer.clear();
        qDebug() << "Port" << m_serialPort->portName() << "został otwarty.";
        return true;
    } else {
        QString errorMsg = "Nie udało się otworzyć portu " + m_serialPort->portName() + ": " + m_serialPort->errorString();
        qWarning() << errorMsg;
        emit errorOccurred(errorMsg);
        return false;
    }
}

void McuAudioReceiver::stop()
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
        qDebug() << "Port" << m_serialPort->portName() << "został zamknięty.";
    }
    m_isActive = false;
}

bool McuAudioReceiver::isActive() const
{
    return m_isActive;
}

void McuAudioReceiver::handleReadyRead()
{
    // zbieranie danych z portu
    m_buffer.append(m_serialPort->readAll());

    constexpr int bytesPerSample = sizeof(int32_t); 
    constexpr int dataBytes = static_cast<int>(samplesPacketSize) * bytesPerSample;
    constexpr int syncBytesSize = 4;
    constexpr int crcBytesSize = 4;
    constexpr int frameSize = syncBytesSize + dataBytes + crcBytesSize;
    
    // Zakładamy, że MCU wysyła bajty w kolejności: 0xAA, 0x55, 0xAA, 0x55
    const QByteArray syncWord = QByteArray::fromHex(SYNC);

    // Dopóki w buforze jest wystarczająco dużo danych na całą ramkę
    while (m_buffer.size() >= frameSize) {
        int syncIndex = m_buffer.indexOf(syncWord);

        if (syncIndex == -1) {
            // Nie znaleziono słowa synchronizującego w dostępnych danych.
            // Zostawiamy tylko ostatnie 3 bajty, bo mogą być początkiem syncWord
            m_buffer.remove(0, m_buffer.size() - 3);
            break;
        }

        if (syncIndex > 0) {
            // Odrzucamy wszystkie śmieci przed znalezionym słowem synchronizującym
            m_buffer.remove(0, syncIndex);
        }

        // Po odcięciu ewentualnych śmieci upewniamy się, czy na pewno mamy kompletną ramkę
        if (m_buffer.size() < frameSize) {
            break;
        }

        // Pobieramy ładunek użyteczny (dane audio) do oddzielnego byteraya
        QByteArray dataPayload = m_buffer.mid(syncBytesSize, dataBytes);
        
        // Wyodrębniamy dołączone CRC z bufora
        uint32_t expectedCrc = 0;
        std::memcpy(&expectedCrc, m_buffer.constData() + syncBytesSize + dataBytes, crcBytesSize);

        // Liczymy sumę z ładunku
        uint32_t calculatedCrc = calculateCRC32(dataPayload);

        if (calculatedCrc == expectedCrc) {
            // Ramka prawidłowa! Rzutujemy bajty na tablicę int32_t
            const int32_t* rawData = reinterpret_cast<const int32_t*>(dataPayload.constData());
            std::vector<int32_t> samples(rawData, rawData + samplesPacketSize);
            emit dataReceived(samples);

            // Zdejmujemy całą poprawną ramkę z bufora
            m_buffer.remove(0, frameSize);
        } else {
            qWarning() << "Błąd sumy kontrolnej CRC32! Oczekiwano:" << expectedCrc << "Obliczono:" << calculatedCrc;
            // Zdejmujemy tylko 1 bajt i szukamy znowu. Jeśli ciąg 0xAA55AA55 wystąpił 
            // w środku paczki audio, ta metoda zagwarantuje szybki powrót do właściwej synchronizacji.
            m_buffer.remove(0, 1);
        }
    }
}

void McuAudioReceiver::handleError(QSerialPort::SerialPortError serialPortError)
{
    if (serialPortError == QSerialPort::ReadError || serialPortError == QSerialPort::ResourceError) {
        QString errorMsg = "Błąd portu szeregowego: " + m_serialPort->errorString();
        qWarning() << errorMsg;
        emit errorOccurred(errorMsg);
        stop();
    }
}
