#pragma once

#include "config.h"
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

    /**
     * @brief Destruktor klasy FFTWidget.
     */
    ~FFTWidget() override = default;

public slots:
    /**
     * @brief Slot przyjmujący gotowe, policzone już widmo amplitudowe w skali decybelowej.
     * @param magnitudes Wektor ze słupkami FFT.
     */
    void updateSpectrum(const std::vector<double>& magnitudes);

    /**
     * @brief Slot aktualizujący dolny przedział wyświetlanego okna częstotliwości.
     * @param min_freq dolny przedział wyświetlanego okna częstotliwości.
     */
    void updateMinFreq(const int& min_freq);

    /**
     * @brief Slot aktualizujący górny przedział wyświetlanego okna częstotliwości.
     * @param max_freq górny przedział wyświetlanego okna częstotliwości.
     */
    void updateMaxFreq(const int& max_freq);

protected:
    /**
     * @brief Rysuje widmo częstotliwościowe na widgecie.
     * @param event Zdarzenie odrysowania (Paint event).
     */
    void paintEvent(QPaintEvent *event) override;

private:
    /**
     * @brief Widmo amplitudowe.
     */
    std::vector<double> m_magnitude;

    /**
     * @brief Timer odświeżania ekranu.
     */
    QTimer *m_refreshTimer;

    /**
     * @brief Flaga informująca, czy mamy już dane do wyświetlenia.
     */
    bool m_hasData = false;
    /**
     * @brief Dolna granica wyświetlanego okna częstotliwości.
     */
    int min_freq = 0;
    /**
     * @brief Górna granica wyświetlanego okna częstotliwości.
     */
    int max_freq = sampleRate / 2;
};
