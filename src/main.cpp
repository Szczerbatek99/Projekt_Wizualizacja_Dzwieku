#include "McuAudioReceiver.h"
#include "OscillogramWidget.h"
#include "FFTWidget.h"
#include "DataProcessor.h"
#include "mainwindow.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qDebug() << "Uruchamianie wizualizera dźwięku...";

    // Odbiornik danych z mikrokontrolera
    McuAudioReceiver receiver;

    // Przetwarzanie danych (FFT, normalizacja itp.)
    DataProcessor processor;

    // Tworzymy główne okno aplikacji
    MainWindow w;
    w.show();

    // Znajdź widgety osadzone w UI i połącz sygnały
    OscillogramWidget *osc = w.findChild<OscillogramWidget*>("oscillogram");
    FFTWidget *fft = w.findChild<FFTWidget*>("fft");

    if (osc) {
        QObject::connect(&processor, &DataProcessor::waveformDataReady,
                         osc, &OscillogramWidget::appendSamples);
    } else {
        qWarning() << "Nie znaleziono widgetu Oscillogram (oscillogram) w MainWindow UI";
    }

    // Podłączenie: nowa paczka próbek -> DataProcessor (dla FFT)
    QObject::connect(&receiver, &McuAudioReceiver::dataReceived,
                     &processor, &DataProcessor::processRawData);

    if (fft) {
        QObject::connect(&processor, &DataProcessor::FFTDataReady,
                         fft, &FFTWidget::updateSpectrum);
    } else {
        qWarning() << "Nie znaleziono widgetu FFT (fft) w MainWindow UI";
    }

    // Obsługa błędów
    QObject::connect(&receiver, &McuAudioReceiver::errorOccurred, [](const QString& errorMsg) {
        qCritical() << "!!! BŁĄD KOMUNIKACJI:" << errorMsg;
    });

    // Start odbioru
    if (!receiver.start()) {
        qCritical() << "Nie udało się wystartować odbiornika.";
    }

    return a.exec();
}