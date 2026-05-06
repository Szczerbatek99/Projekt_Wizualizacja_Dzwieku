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
     * @param magnitudes Wektor ze słupkami FFT.
     */
    void updateSpectrum(const std::vector<double>& magnitudes);

protected:
    /**
     * @brief Rysuje widmo częstotliwościowe na widgecie.
     */
    void paintEvent(QPaintEvent *event) override;

private:
    std::vector<double> m_magnitude;  // Widmo amplitudowe
    QTimer *m_refreshTimer;             // Timer odświeżania ekranu
    bool m_hasData = false;             // Flaga: czy mamy już dane do wyświetlenia
};
