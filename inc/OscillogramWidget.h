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
    ~OscillogramWidget() override = default;
    
    /**
     * @brief Rysuje przebieg audio na widgecie.
     */
    void paintEvent(QPaintEvent *event) override;

public slots:
    /**
     * @brief Slot przyjmujący nową paczkę próbek audio (znormalizowane -1..1).
     * @param data Wektor z próbkami (domyślnie 2048 próbek w paczce).
     */
    void appendSamples(const std::vector<double>& data);


private:
    static constexpr int bufferSize = sampleRate * displaySeconds; // wielkość bufora w bajtach

    std::deque<double> m_buffer;  // Bufor cykliczny na próbki (znormalizowane -1..1)
    QTimer *m_refreshTimer;        // Timer odświeżania ekranu

    // Nie potrzebujemy już autoskalowania osi Y — dane są znormalizowane do [-1,1]

};
