#include "DataProcessor.hh"
#include "config.hh"
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

std::vector<std::complex<double>> DataProcessor::calculateFFT(const std::vector<double>& windowedData)
{
    int N = static_cast<int>(windowedData.size());
    
    int outSize = N / 2 + 1;
    std::vector<std::complex<double>> complexSpectrum(outSize);

    fftw_plan plan = fftw_plan_dft_r2c_1d(N, const_cast<double*>(windowedData.data()),
        reinterpret_cast<fftw_complex*>(complexSpectrum.data()), FFTW_ESTIMATE);

    fftw_execute(plan);
    fftw_destroy_plan(plan);

    return complexSpectrum;
}

void DataProcessor::processRawData(const std::vector<int32_t>& data)
{
    if (static_cast<int>(data.size()) < samplesPacketSize) return;
    std::vector<double> normalizedData(samplesPacketSize);

    // Normalize according to 32-bit signed samples sent by MCU (int32_t)
    // Use 2^31 as denominator so -2^31 maps to -1.0 and +2^31-1 maps slightly below 1.0
    constexpr double scale = 1.0 / 2147483648.0;
    for (size_t i = 0; i < samplesPacketSize; ++i) {
        double v = static_cast<double>(data[i]) * scale;
        if (v > 1.0) v = 1.0;
        else if (v < -1.0) v = -1.0;
        normalizedData[i] = v;
    }

    emit waveformDataReady(normalizedData);

    std::vector<double> windowed = normalizedData;
    applyHannWindow(windowed);

    std::vector<std::complex<double>> complexSpectrum = calculateFFT(windowed);

    int N = static_cast<int>(windowed.size());
    // Basic scale for single-sided spectrum (double-sided -> single-sided): 2/N
    double scale_factor = 2.0 / static_cast<double>(N);
    // Compensate for Hann window coherent gain (~0.5), so we multiply by 1/0.5 = 2
    const double hann_correction = 1.0 / 0.5; // = 2.0
    scale_factor *= hann_correction; // effectively 4.0 / N for Hann window

    std::vector<double> magnitudes;
    magnitudes.reserve(complexSpectrum.size());
    for (size_t k = 0; k < complexSpectrum.size(); ++k) {
        double mag = std::hypot(complexSpectrum[k].real(), complexSpectrum[k].imag()) * scale_factor;
        // DC bin (k==0) and Nyquist (when present, k==N/2) should not be doubled for single-sided spectrum
        if (k == 0 || (N % 2 == 0 && static_cast<int>(k) == N/2)) {
            mag *= 0.5; // undo the doubling for these bins
        }
        magnitudes.push_back(mag);
    }

    emit FFTDataReady(magnitudes);
}
