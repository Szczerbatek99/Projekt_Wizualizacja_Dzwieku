#pragma once

#include <QWidget>
#include <QTimer>
#include <vector>
#include <complex>
#include <cstdint>

/**
 * @brief Widget wyświetlający widmo częstotliwościowe sygnału (FFT).
 */
class FFTWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy FFTWidget.
     * @param parent Wskaźnik na element nadrzędny.
     */
    explicit FFTWidget(QWidget *parent = nullptr);
    ~FFTWidget() override = default;

public slots:
    /**
     * @brief Slot przyjmujący gotowe, policzone już widmo amplitudowe w skali decybelowej.
     * @param magnitudesDb Wektor ze słupkami FFT [dB].
     */
    void updateSpectrum(const std::vector<double>& magnitudesDb);

protected:
    /**
     * @brief Rysuje widmo częstotliwościowe na widgecie.
     */
    void paintEvent(QPaintEvent *event) override;

private:
    static constexpr int FFT_SIZE = 1024;            ///< Rozmiar FFT (2^10)
    static constexpr int SAMPLE_RATE = 16000;        ///< Częstotliwość próbkowania [Hz]

    std::vector<double> m_magnitudeDb;  ///< Widmo amplitudowe w dB
    QTimer *m_refreshTimer;             ///< Timer odświeżania ekranu
    bool m_hasData = false;             ///< Flaga: czy mamy już dane do wyświetlenia
};
