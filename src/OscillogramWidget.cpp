#include "OscillogramWidget.h"
#include <QPainter>
#include <QPen>
#include <QLinearGradient>
#include <algorithm>
#include <cmath>

OscillogramWidget::OscillogramWidget(QWidget *parent)
    : QWidget(parent)
    , m_refreshTimer(new QTimer(this))
{
    setMinimumSize(800, 400);
    // Ten widget jest osadzony w MainWindow; nie ustawiamy tytułu okna.

    // Odświeżanie co ~33ms = ~30 FPS (wystarczająco płynne, a nie obciąża CPU)
    connect(m_refreshTimer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
    m_refreshTimer->start(refreshRate);
}

void OscillogramWidget::appendSamples(const std::vector<double>& data)
{
    // Dorzuć nowe próbki na koniec bufora (dane już znormalizowane w [-1,1])
    for (double sample : data) {
        m_buffer.push_back(sample);
    }

    // Jeśli bufor przekroczył maksymalny rozmiar, usuń najstarsze próbki z przodu
    while (static_cast<int>(m_buffer.size()) > oscillogramBufferSize) {
        m_buffer.pop_front();
    }
}

void OscillogramWidget::paintEvent(QPaintEvent * /*event*/)
{
    // ustawienia do rysowania
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int w = width();
    const int h = height();

    // === Tło ===
    QLinearGradient bgGrad(0, 0, 0, h);
    bgGrad.setColorAt(0.0, QColor(25, 25, 35));
    bgGrad.setColorAt(1.0, QColor(5, 5, 25));
    painter.fillRect(rect(), bgGrad);

    // === Siatka (grid) ===
    painter.setPen(QPen(QColor(40, 40, 60), 1, Qt::DotLine));
    
    constexpr int horizontal_lines = 5;
    constexpr int vertical_lines = 20;

    // Linie poziome (5 linii)
    for (int i = 1; i < horizontal_lines; ++i) {
        int y = h * i / horizontal_lines;
        painter.drawLine(0, y, w, y);
    }

    // Linie pionowe — co 0.25 sekundy (20 linii na 5s)
    for (int i = 1; i < vertical_lines; ++i) {
        int x = w * i / vertical_lines;
        painter.drawLine(x, 0, x, h);
    }

    // === Linia zerowa ===
    painter.setPen(QPen(QColor(80, 80, 100), 1, Qt::SolidLine));
    const int zeroY = h / 2;
    painter.drawLine(0, zeroY, w, zeroY);

    // === Przebieg audio ===
    const int bufSize = static_cast<int>(m_buffer.size());
    if (bufSize < 2) return; // za mało danych

    // Ustalona skala Y: dane znormalizowane w zakresie [-1, 1]
    const double margin = 0.05; // 5% marginesu górnego i dolnego
    const double topMargin = h * margin;
    const double usableH = h * (1.0 - 2.0 * margin);

    // Gradient na linii przebiegu
    QLinearGradient lineGrad(0, 0, 0, h);
    lineGrad.setColorAt(0.0, QColor(0, 220, 180));   // cyjan na górze
    lineGrad.setColorAt(0.5, QColor(0, 180, 255));    // niebieski na środku (zero)
    lineGrad.setColorAt(1.0, QColor(120, 80, 255));   // fioletowy na dole

    QPen wavePen(QBrush(lineGrad), 1.5);
    painter.setPen(wavePen);

    // Decymacja: jeśli mamy więcej próbek niż pikseli szerokości — rysujemy min/max na piksel
    const double samplesPerPixel = static_cast<double>(bufSize) / w;

    if (samplesPerPixel <= 1.0) {
        // Mniej próbek niż pikseli — rysujemy każdą próbkę jako punkt na linii
        QPointF prev;
        for (int i = 0; i < bufSize; ++i) {
            double x = (static_cast<double>(i) / (bufSize - 1)) * w;
            double n = static_cast<double>(m_buffer[i]); // wartość w [-1,1]
            double v = (n + 1.0) * 0.5; // zamiana na [0,1]
            double y = topMargin + (1.0 - v) * usableH;

            QPointF current(x, y);
            if (i > 0) {
                painter.drawLine(prev, current);
            }
            prev = current;
        }
    } else {
        // Więcej próbek niż pikseli — dla każdego piksela szukamy min i max próbki
        // i rysujemy pionową kreskę (envelope), co daje "gęsty" przebieg bez aliasingu
        QPointF prevTop, prevBot;
        for (int px = 0; px < w; ++px) {
            int startSample = static_cast<int>(px * samplesPerPixel);
            int endSample = std::min(static_cast<int>((px + 1) * samplesPerPixel), bufSize);

            double localMin = m_buffer[startSample];
            double localMax = m_buffer[startSample];
            for (int s = startSample + 1; s < endSample; ++s) {
                if (m_buffer[s] < localMin) localMin = m_buffer[s];
                if (m_buffer[s] > localMax) localMax = m_buffer[s];
            }

            double vMax = (localMax + 1.0) * 0.5;
            double vMin = (localMin + 1.0) * 0.5;
            double yTop = topMargin + (1.0 - vMax) * usableH;
            double yBot = topMargin + (1.0 - vMin) * usableH;

            // Rysowanie pionowej kreski min-max
            painter.drawLine(QPointF(px, yTop), QPointF(px, yBot));

            // Łączenie z poprzednim pikselem dla ciągłości
            if (px > 0) {
                painter.drawLine(prevTop, QPointF(px, yTop));
                painter.drawLine(prevBot, QPointF(px, yBot));
            }
            prevTop = QPointF(px, yTop);
            prevBot = QPointF(px, yBot);
        }
    }

    // === Etykiety czasu na osi X ===
    painter.setPen(QColor(140, 140, 160));
    QFont labelFont("monospace", 9);
    painter.setFont(labelFont);

    for (int i = 0; i <= vertical_lines; ++i) {
        double timeSec = (displaySeconds * static_cast<double>(i)) / static_cast<double>(vertical_lines);
        int x = w * i / vertical_lines;
        QString label = QString::number(timeSec, 'f', 2) + "s";
        painter.drawText(x + 2, h - 5, label);
    }
}
