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
    /**
     * @brief Odczytuje dostępne bajty z portu i składa pełne paczki próbek.
     *
     * Metoda buforuje przychodzące bajty w wewnętrznym `QByteArray m_buffer`.
     * Gdy w buforze znajduje się przynajmniej `samplesPacketSize * sizeof(int32_t)` bajtów,
     * wycina jedną paczkę, konwertuje ją na `std::vector<int32_t>` i emituje `dataReceived`.
     */
    void handleReadyRead();

    /**
     * @brief Obsługa błędów portu szeregowego.
     *
     * Emituje `errorOccurred` dla błędów krytycznych i zatrzymuje odbiór w razie potrzeby.
     * @param serialPortError Typ błędu zgłaszany przez QSerialPort.
     */
    void handleError(QSerialPort::SerialPortError serialPortError);

private:
    /**
     * @brief Flaga określająca, czy odbiornik jest aktywny i nasłuchuje na dane.
     */
    bool m_isActive = false;
    
    /**
     * @brief Wskaźnik na port szeregowy używany do komunikacji.
     */
    QSerialPort *m_serialPort;

    /**
     * @brief Bufor gromadzący surowe bajty odebrane z mikrokontrolera.
     */
    QByteArray m_buffer;
};
