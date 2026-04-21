#pragma once

#include <QObject>
#include <QString>
#include <vector>
#include <cstdint>
#include <QSerialPort>
#include <QByteArray>

/**
 * @brief Klasa zarządzająca pobieraniem danych audio z mikrokontrolera.
 */
class McuAudioReceiver : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy McuAudioReceiver.
     * @param parent Wskaźnik na element nadrzędny.
     */
    explicit McuAudioReceiver(QObject *parent = nullptr);

    /**
     * @brief Destruktor klasy McuAudioReceiver.
     */
    ~McuAudioReceiver() override;

    /**
     * @brief Rozpoczyna proces pobierania danych z mikrokontrolera.
     * @return true, jeśli proces wystartował poprawnie, w przeciwnym wypadku false.
     */
    bool start();

    /**
     * @brief Zatrzymuje proces nasłuchiwania / pobierania danych.
     */
    void stop();

    /**
     * @brief Sprawdza, czy odbiornik obecnie pobiera dane.
     * @return true jeśli jest aktywny, false w przeciwnym razie.
     */
    bool isActive() const;

signals:
    /**
     * @brief Sygnał emitowany, gdy nowa "paczka" danych audio zostanie odebrana.
     * @param data Wektor zawierający surowe próbki dźwięku.
     */
    void dataReceived(const std::vector<int32_t>& data);

    /**
     * @brief Sygnał emitowany, gdy napotkano błąd podczas komunikacji.
     * @param errorMessage Tekstowy opis zaistniałego błędu.
     */
    void errorOccurred(const QString& errorMessage);

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError serialPortError);

private:
    // WAŻNE: Przywróciłem 'm_isActive'. Gdy miałeś samo 'isActive' i metodę 'isActive()' C++ wywali błąd kompilacji
    bool m_isActive = false;
    
    QSerialPort *m_serialPort;
    QByteArray m_buffer;
    
    // Rozmiar paczki w liczbie próbek (zmienna 2^N, tutaj początkowo 1024)
    size_t m_currentPacketSamples = 1024; 
};
