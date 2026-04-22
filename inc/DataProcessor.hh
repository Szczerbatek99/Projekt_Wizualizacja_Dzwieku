#pragma once

#include <QObject>
#include <vector>
#include <complex>

/**
 * @brief Klasa odpowiedzialna za przetwarzanie i przygotowywanie danych 
 * dźwiękowych z mikrokontrolera do celów wizualizacji graficznych.
 */
class DataProcessor : public QObject
{
    Q_OBJECT

public:
    explicit DataProcessor(QObject *parent = nullptr);
    ~DataProcessor() override = default;

public slots:
    /**
     * @brief Główny slot odbierający surowe inty prosto z mikrokontrolera.
     * @param data Surowe dane audio 24-bitowe w kontenerze 32-bitowym.
     */
    void processRawData(const std::vector<int32_t>& data);

signals:
    /**
     * @brief Znormalizowany przebieg od -1.0 do 1.0 (do oscyloskopu).
     */
    void waveformDataReady(const std::vector<double>& normalizedData);

    /**
     * @brief Dane z analizy fouriera gotowe do wyświetlenia.
     */
    void FFTDataReady(const std::vector<double>& magnitudes);

private:
    /**
     * @brief Oblicza FFT dla danych rzeczywistych i zwraca widmo zespolone
     * (wynik dla częstotliwości od 0 do Nyquista).
     */
    std::vector<std::complex<double>> calculateFFT(const std::vector<double>& windowedData);

    /**
     * @brief Stosuje okno Hanninga (redukcja wyciekania widma).
     */
    void applyHannWindow(std::vector<double>& data);
};
