#include "McuAudioReceiver.hh"
#include "OscillogramWidget.hh"
#include "FFTWidget.hh"
#include "DataProcessor.hh"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qDebug() << "Uruchamianie wizualizera dźwięku...";

    // Odbiornik danych z mikrokontrolera
    McuAudioReceiver receiver;

    // Okno 1: Oscylogram (przebieg czasowy — 2 sekundy)
    OscillogramWidget oscillogram;
    oscillogram.resize(1024, 400);
    oscillogram.move(50, 50);
    oscillogram.show();

    // Okno 2: Widmo FFT (transformata Fouriera)
    FFTWidget spectrum;
    spectrum.resize(1024, 400);
    spectrum.move(50, 500);
    spectrum.show();

    // Przetwarzanie danych (FFT, normalizacja itp.)
    DataProcessor processor;

    // Podłączenie: nowa paczka próbek -> oscylogram
    QObject::connect(&receiver, &McuAudioReceiver::dataReceived,
                     &oscillogram, &OscillogramWidget::appendSamples);
                     
    // Podłączenie: nowa paczka próbek -> DataProcessor (dla FFT)
    QObject::connect(&receiver, &McuAudioReceiver::dataReceived,
                     &processor, &DataProcessor::processRawData);

    // Podłączenie: gotowe wyniki powrotne po wykonaniu FFT -> wykres widma
    QObject::connect(&processor, &DataProcessor::FFTDataReady,
                     &spectrum, &FFTWidget::updateSpectrum);

    // Obsługa błędów
    QObject::connect(&receiver, &McuAudioReceiver::errorOccurred, [](const QString& errorMsg) {
        qCritical() << "!!! BŁĄD KOMUNIKACJI:" << errorMsg;
    });

    // Start odbioru
    if (!receiver.start()) {
        qCritical() << "Nie udało się wystartować odbiornika. Uruchamiam same okna.";
    }

    return a.exec();
}