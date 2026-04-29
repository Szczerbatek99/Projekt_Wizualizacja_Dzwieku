#pragma once

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
     * @param data Wektor z próbkami (1024 próbki w paczce).
     */
    void appendSamples(const std::vector<double>& data);


private:
    static constexpr int SAMPLE_RATE = 16000;       ///< Częstotliwość próbkowania [Hz]
    static constexpr int DISPLAY_SECONDS = 5;        ///< Czas wyświetlany na ekranie [s]
    static constexpr int BUFFER_SIZE = SAMPLE_RATE * DISPLAY_SECONDS; ///< 32000 próbek

    std::deque<double> m_buffer;  ///< Bufor cykliczny na próbki (znormalizowane -1..1)
    QTimer *m_refreshTimer;        ///< Timer odświeżania ekranu

    // Nie potrzebujemy już autoskalowania osi Y — dane są znormalizowane do [-1,1]

};
