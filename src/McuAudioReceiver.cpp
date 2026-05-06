#include "McuAudioReceiver.h"
#include "config.h"
#include <QDebug>

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
    constexpr int bytesPerPacket = static_cast<int>(samplesPacketSize) * bytesPerSample;

    // sprawdzanie czy dane są przynajmniej rozmiaru bytesPerPacket
    while (m_buffer.size() >= bytesPerPacket) {
        
        // kopiowanie paczki z bufora do zmiennej
        QByteArray packetData = m_buffer.left(bytesPerPacket);
        // usuwanie paczki z bufora
        m_buffer.remove(0, bytesPerPacket);

        // rzutowanie danych z ByteArray do tablicy int32_t
        const int32_t* rawData = reinterpret_cast<const int32_t*>(packetData.constData());
        
        // wpisywanie danych do wektora int32_t
        std::vector<int32_t> samples(rawData, rawData + samplesPacketSize);
        emit dataReceived(samples);
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
