#include "DataProcessor.h"
#include "config.h"
#include <cmath>
#include <algorithm>
#include <fftw3.h>
#include <limits>

DataProcessor::DataProcessor(QObject *parent):
    QObject(parent)
{
    
}

void DataProcessor::applyHannWindow(std::vector<double>& data)
{
    const int N = static_cast<int>(data.size());
    for (int i = 0; i < N; ++i) {
        double window = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / (N - 1)));
        data[i] *= window;
    }
}

std::vector<std::complex<double>> DataProcessor::calculateFFT(const std::vector<double>& Data)
{
    int N = static_cast<int>(Data.size());
    
    int outSize = N / 2 + 1;
    std::vector<std::complex<double>> complexSpectrum(outSize);

    fftw_plan plan = fftw_plan_dft_r2c_1d(N, const_cast<double*>(Data.data()),
        reinterpret_cast<fftw_complex*>(complexSpectrum.data()), FFTW_ESTIMATE);

    fftw_execute(plan);
    fftw_destroy_plan(plan);

    return complexSpectrum;
}

void DataProcessor::processRawData(const std::vector<int32_t>& data)
{
    if (static_cast<int>(data.size()) < samplesPacketSize) return;
    std::vector<double> normalizedData(samplesPacketSize);

    constexpr double scale = 1.0 / 2147483648.0;
    for (size_t i = 0; i < samplesPacketSize; ++i) {
        double v = static_cast<double>(data[i]) * scale;
        if (v > 1.0) v = 1.0;
        else if (v < -1.0) v = -1.0;
        normalizedData[i] = v;
    }

    emit waveformDataReady(normalizedData);

    std::vector<double> windowedData = normalizedData;
    applyHannWindow(windowedData);

    std::vector<std::complex<double>> complexSpectrum = calculateFFT(windowedData);

    int N = static_cast<int>(windowedData.size());
    
    // Mnożnik dla widma jednostronnego z normalizacją
    double scale_factor = 2.0 / static_cast<double>(N);
    // Mnożnik korekcji dla okienkowania
    const double hann_correction = 1.0 / 0.5;
    scale_factor *= hann_correction;

    std::vector<double> magnitudes;
    magnitudes.reserve(complexSpectrum.size());

    for (size_t k = 0; k < complexSpectrum.size(); ++k) {
        double mag = std::hypot(complexSpectrum[k].real(), complexSpectrum[k].imag()) * scale_factor;
        // Prążek składowej stałej (DC, k==0) oraz prążek Nyquista (jeśli występuje, k==N/2) 
        // nie powinny być podwajane w widmie jednostronnym
        if (k == 0 || (N % 2 == 0 && static_cast<int>(k) == N/2)) {
            mag *= 0.5; // cofnięcie podwojenia dla tych prążków
        }
        magnitudes.push_back(mag);
    }

    emit FFTDataReady(magnitudes);
}
