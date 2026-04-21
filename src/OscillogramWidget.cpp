#include "OscillogramWidget.hh"
#include <QPainter>
#include <QPen>
#include <QLinearGradient>
#include <algorithm>
#include <cmath>

OscillogramWidget::OscillogramWidget(QWidget *parent)
    : QWidget(parent)
    , m_refreshTimer(new QTimer(this))
{
    // Ciemne tło okna
    setMinimumSize(800, 400);
    setWindowTitle("Przebieg czasowy sygnału audio");

    // Odświeżanie co ~33ms = ~30 FPS (wystarczająco płynne, a nie obciąża CPU)
    connect(m_refreshTimer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
    m_refreshTimer->start(33);
}

void OscillogramWidget::appendSamples(const std::vector<int32_t>& data)
{
    // Dorzuć nowe próbki na koniec bufora
    for (int32_t sample : data) {
        m_buffer.push_back(sample);
    }

    // Jeśli bufor przekroczył 2 sekundy, usuń najstarsze próbki z przodu
    while (static_cast<int>(m_buffer.size()) > BUFFER_SIZE) {
        m_buffer.pop_front();
    }

    // Aktualizuj wartości szczytowe do autoskalowania osi Y
    // (nie szukamy min/max w CAŁYM buforze co paczkę — tylko sprawdzamy nową paczkę
    //  i powoli "wygaszamy" stare szczyty, aby wykres adaptował się do głośności)
    for (int32_t sample : data) {
        if (sample > m_peakMax) m_peakMax = sample;
        if (sample < m_peakMin) m_peakMin = sample;
    }

    // Powolne wygaszanie szczytów (decay) — żeby oś Y nie "wariowała"
    // przy nagłej ciszy po głośnym dźwięku
    m_peakMax = static_cast<int32_t>(m_peakMax * 0.999);
    m_peakMin = static_cast<int32_t>(m_peakMin * 0.999);

    // Minimalna rozpiętość, żeby nie dzielić przez zero
    if (m_peakMax - m_peakMin < 2) {
        m_peakMax = 1;
        m_peakMin = -1;
    }
}

void OscillogramWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int w = width();
    const int h = height();

    // === Tło ===
    QLinearGradient bgGrad(0, 0, 0, h);
    bgGrad.setColorAt(0.0, QColor(15, 15, 25));
    bgGrad.setColorAt(1.0, QColor(5, 5, 15));
    painter.fillRect(rect(), bgGrad);

    // === Siatka (grid) ===
    painter.setPen(QPen(QColor(40, 40, 60), 1, Qt::DotLine));

    // Linie poziome (5 linii)
    for (int i = 1; i < 5; ++i) {
        int y = h * i / 5;
        painter.drawLine(0, y, w, y);
    }

    // Linie pionowe — co 0.25 sekundy (8 linii na 2s)
    for (int i = 1; i < 8; ++i) {
        int x = w * i / 8;
        painter.drawLine(x, 0, x, h);
    }

    // === Linia zerowa ===
    painter.setPen(QPen(QColor(80, 80, 100), 1, Qt::SolidLine));
    const int zeroY = h / 2;
    painter.drawLine(0, zeroY, w, zeroY);

    // === Przebieg audio ===
    const int bufSize = static_cast<int>(m_buffer.size());
    if (bufSize < 2) return; // za mało danych

    // Rozpiętość Y
    const double range = static_cast<double>(m_peakMax) - static_cast<double>(m_peakMin);
    const double margin = 0.05; // 5% marginesu górnego i dolnego
    const double usableH = h * (1.0 - 2.0 * margin);
    const double topMargin = h * margin;

    // Gradient na linii przebiegu
    QLinearGradient lineGrad(0, 0, 0, h);
    lineGrad.setColorAt(0.0, QColor(0, 220, 180));   // cyjan na górze
    lineGrad.setColorAt(0.5, QColor(0, 180, 255));    // niebieski na środku (zero)
    lineGrad.setColorAt(1.0, QColor(120, 80, 255));   // fioletowy na dole

    QPen wavePen(QBrush(lineGrad), 1.5);
    painter.setPen(wavePen);

    // Decimacja: jeśli mamy więcej próbek niż pikseli szerokości — rysujemy min/max na piksel
    const double samplesPerPixel = static_cast<double>(bufSize) / w;

    if (samplesPerPixel <= 1.0) {
        // Mniej próbek niż pikseli — rysujemy każdą próbkę jako punkt na linii
        QPointF prev;
        for (int i = 0; i < bufSize; ++i) {
            double x = (static_cast<double>(i) / (bufSize - 1)) * w;
            double normalized = (static_cast<double>(m_buffer[i]) - m_peakMin) / range;
            double y = topMargin + usableH * (1.0 - normalized);

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

            int32_t localMin = m_buffer[startSample];
            int32_t localMax = m_buffer[startSample];
            for (int s = startSample + 1; s < endSample; ++s) {
                if (m_buffer[s] < localMin) localMin = m_buffer[s];
                if (m_buffer[s] > localMax) localMax = m_buffer[s];
            }

            double normMin = (static_cast<double>(localMin) - m_peakMin) / range;
            double normMax = (static_cast<double>(localMax) - m_peakMin) / range;
            double yTop = topMargin + usableH * (1.0 - normMax);
            double yBot = topMargin + usableH * (1.0 - normMin);

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

    for (int i = 0; i <= 8; ++i) {
        double timeSec = (DISPLAY_SECONDS * static_cast<double>(i)) / 8.0;
        int x = w * i / 8;
        QString label = QString::number(timeSec, 'f', 2) + "s";
        painter.drawText(x + 2, h - 5, label);
    }
}
