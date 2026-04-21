#include "McuAudioReceiver.hh"
#include <QDebug>

McuAudioReceiver::McuAudioReceiver(QObject *parent)
    : QObject(parent)
    , m_isActive(false)
    , m_serialPort(new QSerialPort(this))
{
    // Podłączamy sygnały portu do wewnętrznych slotów tej klasy
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

    // Hardkodowane ustawienia dla malinki (w WSL2 domyślnie jest to /dev/ttyUSB0 lub /dev/ttyACM0)
    m_serialPort->setPortName("/dev/ttyACM0");
    
    // Często przy strumieniowaniu audio z malinki po UART ustawia się najszybszy sensowny baudrate (dostosuj później w razie potrzeby)
    m_serialPort->setBaudRate(115200); 
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
    // Dodajemy nowo przybyłe bajty na koniec naszego bufora
    m_buffer.append(m_serialPort->readAll());

    // Każda próbka to 32 bity, czyli 4 bajty
    const int bytesPerSample = sizeof(int32_t); 
    // Chwilowo mamy sztywną wartość 1024, więc paczka musi zajmować 4096 bajtów
    const int bytesPerPacket = static_cast<int>(m_currentPacketSamples) * bytesPerSample;

    // Możliwe, że przyszło na raz więcej bajtów niż jedna paczka. 
    // Pętla 'while' na to pozwala:
    while (m_buffer.size() >= bytesPerPacket) {
        
        // Wyodrębnij z początku bufora bajty tylko dla JEDNEJ paczki
        QByteArray packetData = m_buffer.left(bytesPerPacket);
        
        // Wyrzuć te bajty z głównego bufora
        m_buffer.remove(0, bytesPerPacket);

        // Skopiuj te czyste binarne dane prosto na stos do int32_t.
        // Zakładamy, że bajty przylatują w kolejności zgodnej z architekturą Twojego PC (z reguły little-endian)
        const int32_t* rawData = reinterpret_cast<const int32_t*>(packetData.constData());
        
        // Tworzymy finalny std::vector i przerzucamy w niego te 1024 próbki
        std::vector<int32_t> samples(rawData, rawData + m_currentPacketSamples);

        // Nadajemy światu komunikat o nowej paczce
        emit dataReceived(samples);
    }
}

void McuAudioReceiver::handleError(QSerialPort::SerialPortError serialPortError)
{
    // Reaguj tylko na rzeczywiste błędy sprzętowe czy odłączenie malinki z USB
    if (serialPortError == QSerialPort::ReadError || serialPortError == QSerialPort::ResourceError) {
        QString errorMsg = "Błąd portu szeregowego: " + m_serialPort->errorString();
        qWarning() << errorMsg;
        emit errorOccurred(errorMsg);
        stop();
    }
}
