#pragma once

#include <QWidget>
#include <QTimer>
#include <vector>
#include <cstdint>
#include <deque>

/**
 * @brief Widget wyświetlający przebieg czasowy sygnału audio.
 *
 * Przechowuje bufor cykliczny na 2 sekundy danych (przy 16kHz = 32000 próbek).
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

public slots:
    /**
     * @brief Slot przyjmujący nową paczkę próbek audio.
     * @param data Wektor z próbkami (1024 próbki w paczce).
     */
    void appendSamples(const std::vector<int32_t>& data);

protected:
    /**
     * @brief Rysuje przebieg audio na widgecie.
     */
    void paintEvent(QPaintEvent *event) override;

private:
    static constexpr int SAMPLE_RATE = 16000;       ///< Częstotliwość próbkowania [Hz]
    static constexpr int DISPLAY_SECONDS = 2;        ///< Czas wyświetlany na ekranie [s]
    static constexpr int BUFFER_SIZE = SAMPLE_RATE * DISPLAY_SECONDS; ///< 32000 próbek

    std::deque<int32_t> m_buffer;  ///< Bufor cykliczny na próbki
    QTimer *m_refreshTimer;        ///< Timer odświeżania ekranu

    int32_t m_peakMax = 1;         ///< Bieżąca wartość szczytowa (do autoskalowania osi Y)
    int32_t m_peakMin = -1;        ///< Bieżąca wartość minimalna
};
