#pragma once

#include "config.h"
#include <QWidget>
#include <QTimer>
#include <vector>
#include <cstdint>
#include <deque>

/**
 * @brief Widget wyświetlający przebieg czasowy sygnału audio.
 *
 * Przechowuje bufor cykliczny na 5 sekund danych (przy 16kHz = 80 000 próbek).
 * Rysuje przebieg za pomocą QPainter w paintEvent.
 */
class OscillogramWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy OscillogramWidget.
     * @param parent Wskaźnik na element nadrzędny.
     */
    explicit OscillogramWidget(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy OscillogramWidget.
     */
    ~OscillogramWidget() override = default;
    
    /**
     * @brief Rysuje przebieg audio na widgecie.
     * @param event Zdarzenie odrysowania (Paint event).
     */
    void paintEvent(QPaintEvent *event) override;

public slots:
    /**
     * @brief Slot przyjmujący nową paczkę próbek audio (znormalizowane -1..1).
     * @param data Wektor z próbkami.
     */
    void appendSamples(const std::vector<double>& data);


private:
    /**
     * @brief Bufor cykliczny na próbki (znormalizowane od -1.0 do 1.0).
     */
    std::deque<double> m_buffer;

    /**
     * @brief Timer odpowiedzialny za odświeżanie ekranu.
     */
    QTimer *m_refreshTimer;
};
