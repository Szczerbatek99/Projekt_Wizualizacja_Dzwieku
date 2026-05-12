#pragma once

#include "config.h"
#include <QWidget>
#include <QTimer>
#include <QImage>
#include <vector>
#include <deque>

/**
 * @brief Widget wyświetlający spektrogram sygnału audio w czasie rzeczywistym.
 *
 * Spektrogram przedstawia widmo częstotliwościowe zmieniające się w czasie:
 * oś X to czas, oś Y to częstotliwość, a kolor piksela koduje amplitudę.
 * Widget przechowuje historię widm FFT w buforze kołowym i renderuje je
 * jako obraz QImage dla maksymalnej wydajności.
 */
class SpectrogramWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy SpectrogramWidget.
     * @param parent Wskaźnik na element nadrzędny.
     */
    explicit SpectrogramWidget(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy SpectrogramWidget.
     */
    ~SpectrogramWidget() override = default;

public slots:
    /**
     * @brief Slot przyjmujący nowe widmo amplitudowe z DataProcessor.
     *
     * Każde wywołanie dodaje jedną pionową kolumnę do spektrogramu.
     * @param magnitudes Wektor amplitud (wynik FFT jednostronnego).
     */
    void appendSpectrum(const std::vector<double>& magnitudes);

    /**
     * @brief Ustawia czas wyświetlanej historii spektrogramu.
     * @param seconds Ilość sekund historii do wyświetlenia.
     */
    void setDisplaySeconds(int seconds);

protected:
    /**
     * @brief Rysuje spektrogram na widgecie.
     * @param event Zdarzenie odrysowania (Paint event).
     */
    void paintEvent(QPaintEvent *event) override;

private:
    /**
     * @brief Konwertuje wartość amplitudy na kolor w palecie spektrogramu.
     *
     * Mapuje amplitudę liniową na skalę kolorów od ciemnego granatu
     * (cisza) przez cyjan i żółty do białego (maksymalna głośność).
     * @param value Znormalizowana wartość amplitudy w zakresie [0.0, 1.0].
     * @return Kolor w formacie QRgb.
     */
    QRgb amplitudeToColor(double value) const;

    /**
     * @brief Renderuje pojedynczą kolumnę widma w obrazie spektrogramu.
     * @param col Indeks kolumny w obrazie.
     * @param spectrum Wektor amplitud do wyrenderowania.
     */
    void renderColumn(int col, const std::vector<double>& spectrum);

    /**
     * @brief Bufor kołowy przechowujący historię widm FFT.
     *
     * Każdy element to jeden wektor amplitud (jedna kolumna spektrogramu).
     */
    std::deque<std::vector<double>> m_history;

    /**
     * @brief Maksymalna liczba kolumn w historii (zależna od czasu wyświetlania).
     *
     * Obliczana jako: displaySeconds * (sampleRate / samplesPacketSize).
     */
    int m_maxColumns;

    /**
     * @brief Aktualny czas wyświetlanej historii w sekundach.
     */
    int m_displaySeconds;

    /**
     * @brief Timer odpowiedzialny za odświeżanie ekranu.
     */
    QTimer *m_refreshTimer;

    /**
     * @brief Obraz renderowany w pamięci przed wyświetleniem.
     *
     * Używany zamiast rysowania pojedynczych prostokątów dla wydajności.
     */
    QImage m_spectrogramImage;

    /**
     * @brief Bieżące maksimum amplitudy używane do normalizacji kolorów.
     */
    double m_ampMax = 0.0;

    /**
     * @brief Liczba binów częstotliwości (wysokość obrazu źródłowego).
     */
    int m_freqBins = 0;

    /**
     * @brief Flaga wymuszająca pełną przebudowę obrazu (np. po zmianie skali amplitudy).
     */
    bool m_needsFullRebuild = false;
};
