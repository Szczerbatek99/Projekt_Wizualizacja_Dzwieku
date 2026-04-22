#include "McuAudioReceiver.hh"
#include "config.hh"
#include <QDebug>

McuAudioReceiver::McuAudioReceiver(QObject *parent):
    QObject(parent), m_isActive(false), m_serialPort(new QSerialPort(this))
{
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

    m_serialPort->setPortName("/dev/ttyACM0");
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
    m_buffer.append(m_serialPort->readAll());

    constexpr int bytesPerSample = sizeof(int32_t); 
    constexpr int bytesPerPacket = static_cast<int>(samplesPacketSize) * bytesPerSample;

    while (m_buffer.size() >= bytesPerPacket) {
        
        QByteArray packetData = m_buffer.left(bytesPerPacket);
        m_buffer.remove(0, bytesPerPacket);

        const int32_t* rawData = reinterpret_cast<const int32_t*>(packetData.constData());
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
