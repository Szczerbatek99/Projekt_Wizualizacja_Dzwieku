#include "FFTWidget.hh"
#include <QPainter>
#include <QPen>
#include <QLinearGradient>
#include <cmath>
#include <algorithm>

FFTWidget::FFTWidget(QWidget *parent)
    : QWidget(parent)
    , m_magnitudeDb(FFT_SIZE / 2, -120.0)
    , m_refreshTimer(new QTimer(this))
{
    setMinimumSize(800, 400);
    setWindowTitle("Widmo częstotliwościowe (FFT)");

    // Odświeżanie ~30 FPS
    connect(m_refreshTimer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
    m_refreshTimer->start(33);
}

void FFTWidget::applyHannWindow(std::vector<double>& data)
{
    const int N = static_cast<int>(data.size());
    for (int i = 0; i < N; ++i) {
        double window = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / (N - 1)));
        data[i] *= window;
    }
}

void FFTWidget::fft(std::vector<std::complex<double>>& data)
{
    const int N = static_cast<int>(data.size());
    if (N <= 1) return;

    // Bit-reversal permutation
    for (int i = 1, j = 0; i < N; ++i) {
        int bit = N >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) std::swap(data[i], data[j]);
    }

    // Cooley-Tukey iterative FFT
    for (int len = 2; len <= N; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        std::complex<double> wBase(std::cos(angle), std::sin(angle));

        for (int i = 0; i < N; i += len) {
            std::complex<double> w(1.0, 0.0);
            for (int j = 0; j < len / 2; ++j) {
                std::complex<double> u = data[i + j];
                std::complex<double> v = data[i + j + len / 2] * w;
                data[i + j] = u + v;
                data[i + j + len / 2] = u - v;
                w *= wBase;
            }
        }
    }
}

void FFTWidget::processSamples(const std::vector<int32_t>& data)
{
    if (static_cast<int>(data.size()) < FFT_SIZE) return;

    // Konwersja int32 -> double (normalizacja do zakresu [-1, 1])
    // Dane 24-bitowe wyrównane do lewej w 32-bitowej zmiennej (LSB = 0)
    // Dzielimy przez 2^31 aby uzyskać zakres [-1, 1]
    std::vector<double> realData(FFT_SIZE);
    for (int i = 0; i < FFT_SIZE; ++i) {
        realData[i] = static_cast<double>(data[i]) / 2147483648.0;
    }

    // Okno Hanninga — redukcja "wyciekania" widmowego (spectral leakage)
    applyHannWindow(realData);

    // Przygotowanie danych zespolonych (część urojona = 0)
    std::vector<std::complex<double>> complexData(FFT_SIZE);
    for (int i = 0; i < FFT_SIZE; ++i) {
        complexData[i] = std::complex<double>(realData[i], 0.0);
    }

    // Obliczenie FFT
    fft(complexData);

    // Obliczenie widma amplitudowego w dB (tylko połowa — symetria FFT)
    const int halfN = FFT_SIZE / 2;
    for (int i = 0; i < halfN; ++i) {
        double magnitude = std::abs(complexData[i]) / halfN;
        // Konwersja na dB (z zabezpieczeniem przed log(0))
        double db = 20.0 * std::log10(std::max(magnitude, 1e-10));
        // Wygładzanie (exponential moving average) — żeby słupki nie skakały zbyt gwałtownie
        m_magnitudeDb[i] = m_magnitudeDb[i] * 0.7 + db * 0.3;
    }

    m_hasData = true;
}

void FFTWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int w = width();
    const int h = height();

    // === Tło ===
    QLinearGradient bgGrad(0, 0, 0, h);
    bgGrad.setColorAt(0.0, QColor(10, 10, 30));
    bgGrad.setColorAt(1.0, QColor(5, 5, 15));
    painter.fillRect(rect(), bgGrad);

    // === Siatka ===
    painter.setPen(QPen(QColor(40, 40, 60), 1, Qt::DotLine));

    // Linie poziome — co 20 dB
    const double dbMin = -120.0;
    const double dbMax = 0.0;
    const double dbRange = dbMax - dbMin;

    for (int db = -100; db <= 0; db += 20) {
        double normalized = (db - dbMin) / dbRange;
        int y = h - static_cast<int>(normalized * h);
        painter.drawLine(0, y, w, y);

        // Etykieta dB
        painter.setPen(QColor(120, 120, 140));
        painter.drawText(5, y - 3, QString::number(db) + " dB");
        painter.setPen(QPen(QColor(40, 40, 60), 1, Qt::DotLine));
    }

    // Linie pionowe — na częstotliwościach kluczowych
    const double nyquist = SAMPLE_RATE / 2.0;
    const int freqMarkers[] = {100, 500, 1000, 2000, 4000, 6000, 8000};
    for (int freq : freqMarkers) {
        double normalized = static_cast<double>(freq) / nyquist;
        int x = static_cast<int>(normalized * w);
        painter.drawLine(x, 0, x, h);

        painter.setPen(QColor(120, 120, 140));
        QString label = (freq >= 1000) ? QString::number(freq / 1000) + "kHz" : QString::number(freq) + "Hz";
        painter.drawText(x + 3, h - 8, label);
        painter.setPen(QPen(QColor(40, 40, 60), 1, Qt::DotLine));
    }

    if (!m_hasData) return;

    // === Rysowanie widma ===
    const int halfN = FFT_SIZE / 2;

    // Gradient koloru słupków
    QLinearGradient barGrad(0, h, 0, 0);
    barGrad.setColorAt(0.0, QColor(0, 80, 200));      // niebieski na dole
    barGrad.setColorAt(0.5, QColor(0, 200, 150));      // cyjan w środku
    barGrad.setColorAt(0.85, QColor(255, 200, 0));     // żółty wysoko
    barGrad.setColorAt(1.0, QColor(255, 50, 50));      // czerwony na szczycie

    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(barGrad));

    // Rysowanie: każdy bin FFT jako pionowy słupek
    const double binWidth = static_cast<double>(w) / halfN;

    for (int i = 1; i < halfN; ++i) { // pomijamy bin 0 (DC offset)
        double normalized = (m_magnitudeDb[i] - dbMin) / dbRange;
        normalized = std::clamp(normalized, 0.0, 1.0);

        int barHeight = static_cast<int>(normalized * h);
        int x = static_cast<int>(i * binWidth);
        int barW = std::max(static_cast<int>(binWidth), 1);

        painter.drawRect(x, h - barHeight, barW, barHeight);
    }
}
