#include "DataProcessor.hh"
#include "config.hh"
#include <cmath>
#include <algorithm>
#include <fftw3.h>

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

    constexpr double scale = 1.0 / 8388607.0;
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

    double scale_factor = 2.0 / windowed.size();

    std::vector<double> magnitudes;
    magnitudes.reserve(complexSpectrum.size());
    for (const auto &c : complexSpectrum) {
        double mag = std::hypot(c.real(), c.imag()) * scale_factor;
        double db = (mag > 1e-6) ? 20.0 * std::log10(mag) : -120.0;
        magnitudes.push_back(db);
    }

    emit FFTDataReady(magnitudes);
}
