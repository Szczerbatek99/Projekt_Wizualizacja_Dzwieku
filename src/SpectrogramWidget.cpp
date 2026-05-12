#include "SpectrogramWidget.h"
#include <QPainter>
#include <QPen>
#include <QLinearGradient>
#include <cmath>
#include <algorithm>
#include <cstring>

SpectrogramWidget::SpectrogramWidget(QWidget *parent)
    : QWidget(parent)
    , m_displaySeconds(displaySeconds)
    , m_refreshTimer(new QTimer(this))
{
    setMinimumSize(400, 200);

    // Obliczenie maksymalnej liczby kolumn na podstawie czasu wyświetlania
    // Przy 16kHz i paczce 256 próbek = 62.5 widm na sekundę
    m_maxColumns = m_displaySeconds * (sampleRate / static_cast<int>(samplesPacketSize));

    // Odświeżanie używające globalnego czasu odświeżania z config.h
    connect(m_refreshTimer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
    m_refreshTimer->start(refreshRate);
}

void SpectrogramWidget::renderColumn(int col, const std::vector<double>& spectrum)
{
    if (m_freqBins == 0) return;

    // Stały próg amplitudy (wartość powyżej której jest biały kolor)
    const double threshold = 0.1;

    for (int row = 0; row < m_freqBins; ++row) {
        int freqIdx = m_freqBins - 1 - row;

        double amplitude = 0.0;
        if (freqIdx < static_cast<int>(spectrum.size())) {
            amplitude = spectrum[freqIdx] / threshold;
        }

        QRgb* line = reinterpret_cast<QRgb*>(m_spectrogramImage.scanLine(row));
        line[col] = amplitudeToColor(amplitude);
    }
}

void SpectrogramWidget::appendSpectrum(const std::vector<double>& magnitudes)
{
    // Dodajemy nowe widmo na koniec historii
    m_history.push_back(magnitudes);

    // Utrzymujemy stały rozmiar bufora kołowego
    while (static_cast<int>(m_history.size()) > m_maxColumns) {
        m_history.pop_front();
    }

    // Ustalenie liczby binów częstotliwości (przy pierwszym widmie)
    if (m_freqBins == 0 && !magnitudes.empty()) {
        m_freqBins = static_cast<int>(magnitudes.size());
        m_spectrogramImage = QImage(m_maxColumns, m_freqBins, QImage::Format_RGB32);
        m_spectrogramImage.fill(qRgb(5, 5, 25)); // Kolor spodu tła
    }

    if (m_needsFullRebuild) {
        // Pełna przebudowa
        const int histSize = static_cast<int>(m_history.size());

        if (m_spectrogramImage.width() != m_maxColumns || m_spectrogramImage.height() != m_freqBins) {
            m_spectrogramImage = QImage(m_maxColumns, m_freqBins, QImage::Format_RGB32);
        }
        m_spectrogramImage.fill(qRgb(5, 5, 25));

        for (int i = 0; i < histSize; ++i) {
            int col = m_maxColumns - histSize + i;
            renderColumn(col, m_history[i]);
        }
        m_needsFullRebuild = false;
    } else {
        // Przyrostowe budowanie: przesuń cały obraz o 1 piksel w lewo, dorysuj nową kolumnę
        const int bytesPerLine = m_spectrogramImage.bytesPerLine();
        for (int row = 0; row < m_freqBins; ++row) {
            uchar* line = m_spectrogramImage.scanLine(row);
            // Przesunięcie wiersza o 1 piksel (4 bajty) w lewo za pomocą memmove
            std::memmove(line, line + sizeof(QRgb), bytesPerLine - sizeof(QRgb));
        }

        // Renderowanie nowej kolumny na prawej krawędzi
        renderColumn(m_maxColumns - 1, magnitudes);
    }
}

void SpectrogramWidget::setDisplaySeconds(int seconds)
{
    if (seconds < 1) seconds = 1;
    if (seconds > 30) seconds = 30;

    m_displaySeconds = seconds;
    m_maxColumns = m_displaySeconds * (sampleRate / static_cast<int>(samplesPacketSize));

    // Przycinamy historię, jeśli zmniejszono okno czasowe
    while (static_cast<int>(m_history.size()) > m_maxColumns) {
        m_history.pop_front();
    }

    // Wymuszamy pełną przebudowę obrazu z nowym rozmiarem
    m_needsFullRebuild = true;
}

QRgb SpectrogramWidget::amplitudeToColor(double value) const
{
    value = std::clamp(value, 0.0, 1.0);

    // Nowa paleta Heatmap: 
    // 0.0   - Ciemny Fiolet (Tło)
    // 0.2   - Niebieski
    // 0.4   - Zielony
    // 0.6   - Żółty
    // 0.8   - Czerwony
    // 1.0   - Biały

    int r, g, b;
    if (value < 0.01) {
        r = 25;
        g = 25;
        b = 35;
    } else if (value < 0.2) {
        double t = value / 0.2;
        r = static_cast<int>(40 + t * (0 - 40));
        g = static_cast<int>(40 + t * (0 - 40));
        b = static_cast<int>(80 + t * (255 - 80));
    } else if (value < 0.4) {
        double t = (value - 0.2) / 0.2;
        r = 0;
        g = static_cast<int>(t * 255);
        b = static_cast<int>(255 - t * 255);
    } else if (value < 0.6) {
        double t = (value - 0.4) / 0.2;
        r = static_cast<int>(t * 255);
        g = 255;
        b = 0;
    } else if (value < 0.8) {
        double t = (value - 0.6) / 0.2;
        r = 255;
        g = static_cast<int>(255 - t * 255);
        b = 0;
    } else {
        double t = (value - 0.8) / 0.2;
        r = 255;
        g = static_cast<int>(t * 255);
        b = static_cast<int>(t * 255);
    }

    return qRgb(r, g, b);
}

void SpectrogramWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    const int w = width();
    const int h = height();

    // === Tło ===
    QLinearGradient bgGrad(0, 0, 0, h);
    bgGrad.setColorAt(0.0, QColor(25, 25, 35));
    bgGrad.setColorAt(1.0, QColor(5, 5, 25));
    painter.fillRect(rect(), bgGrad);

    if (m_history.size() < 2 || m_freqBins == 0) return;

    // === Rysowanie przeskalowanego, gotowego obrazu na widgecie ===
    constexpr int leftMargin = 55;
    constexpr int bottomMargin = 25;
    const int plotW = w - leftMargin;
    const int plotH = h - bottomMargin;

    QImage scaled = m_spectrogramImage.scaled(plotW, plotH, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    painter.drawImage(leftMargin, 0, scaled);

    // === Siatka i etykiety osi Y (częstotliwość) ===
    QFont labelFont("monospace", 9);
    painter.setFont(labelFont);

    const double nyquist = sampleRate / 2.0;
    const int freqMarkers[] = {100, 500, 1000, 2000, 4000, 6000};
    for (int freq : freqMarkers) {
        if (freq > nyquist) continue;
        double normalized = static_cast<double>(freq) / nyquist;
        int y = plotH - static_cast<int>(normalized * plotH);

        painter.setPen(QPen(QColor(80, 80, 100, 120), 1, Qt::DotLine));
        painter.drawLine(leftMargin, y, w, y);

        painter.setPen(QColor(160, 160, 180));
        QString label = (freq >= 1000) ? QString::number(freq / 1000) + "kHz" : QString::number(freq) + "Hz";
        painter.drawText(2, y + 4, label);
    }

    // === Etykiety osi X (czas) ===
    constexpr int timeMarkers = 10;
    for (int i = 0; i <= timeMarkers; ++i) {
        double timeSec = (m_displaySeconds * static_cast<double>(i)) / static_cast<double>(timeMarkers);
        int x = leftMargin + (plotW * i / timeMarkers);

        painter.setPen(QPen(QColor(80, 80, 100, 120), 1, Qt::DotLine));
        painter.drawLine(x, 0, x, plotH);

        painter.setPen(QColor(160, 160, 180));
        QString label = QString::number(timeSec, 'f', 1) + "s";
        painter.drawText(x + 2, h - 5, label);
    }
}
