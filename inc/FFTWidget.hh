#pragma once

#include <QWidget>
#include <QTimer>
#include <vector>
#include <complex>
#include <cstdint>

/**
 * @brief Widget wyświetlający widmo częstotliwościowe sygnału (FFT).
 *
 * Przyjmuje paczki 1024 próbek, oblicza FFT (Cooley-Tukey radix-2)
 * i rysuje widmo amplitudowe w skali logarytmicznej (dB).
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
     * @brief Slot przyjmujący nową paczkę próbek audio do analizy FFT.
     * @param data Wektor z próbkami (1024 próbki w paczce).
     */
    void processSamples(const std::vector<int32_t>& data);

protected:
    /**
     * @brief Rysuje widmo częstotliwościowe na widgecie.
     */
    void paintEvent(QPaintEvent *event) override;

private:
    static constexpr int FFT_SIZE = 1024;            ///< Rozmiar FFT (2^10)
    static constexpr int SAMPLE_RATE = 16000;        ///< Częstotliwość próbkowania [Hz]

    /**
     * @brief Oblicza FFT in-place (Cooley-Tukey radix-2).
     * @param data Wektor liczb zespolonych (wejście/wyjście).
     */
    void fft(std::vector<std::complex<double>>& data);

    /**
     * @brief Stosuje okno Hanninga na dane wejściowe (redukcja spectral leakage).
     * @param data Wektor próbek do przetworzenia.
     */
    void applyHannWindow(std::vector<double>& data);

    std::vector<double> m_magnitudeDb;  ///< Widmo amplitudowe w dB (FFT_SIZE/2 elementów)
    QTimer *m_refreshTimer;             ///< Timer odświeżania ekranu
    bool m_hasData = false;             ///< Flaga: czy mamy już dane do wyświetlenia
};
