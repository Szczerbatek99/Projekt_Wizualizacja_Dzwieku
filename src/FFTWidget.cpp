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

void FFTWidget::updateSpectrum(const std::vector<double>& magnitudesDb)
{
    // Kopiujemy gotowe decybele do naszego bufora na wypadek gdyby
    // repaint() wywołał się po powrocie ze slota
    m_magnitudeDb = magnitudesDb;
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
    const int halfN = m_magnitudeDb.size();
    if (halfN == 0) return;

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
