#include "McuAudioReceiver.hh"
#include "OscillogramWidget.hh"
#include "FFTWidget.hh"
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

    // Okno 2: Widmo FFT (transformata Fouriera — 1024 próbki)
    FFTWidget spectrum;
    spectrum.resize(1024, 400);
    spectrum.move(50, 500);
    spectrum.show();

    // Podłączenie: nowa paczka próbek -> oba okna jednocześnie
    QObject::connect(&receiver, &McuAudioReceiver::dataReceived,
                     &oscillogram, &OscillogramWidget::appendSamples);
    QObject::connect(&receiver, &McuAudioReceiver::dataReceived,
                     &spectrum, &FFTWidget::processSamples);

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
