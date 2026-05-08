#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "OscillogramWidget.h"
#include "FFTWidget.h"

MainWindow::MainWindow(QWidget *parent):
     QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->Widoki_sw->setCurrentWidget(ui->MainMenu_w);
    ui->Widoki_an_sw->setCurrentWidget(ui->Osc_Spekt_w);
    ui->Widoki_muz_sw->setCurrentWidget(ui->Pianino_w);

    thread = new QThread(this);

    receiver = new McuAudioReceiver();
    processor = new DataProcessor();
    
    receiver->moveToThread(thread);
    processor->moveToThread(thread);

    // Podłączenie: nowa paczka próbek -> DataProcessor (dla FFT)
    QObject::connect(receiver, &McuAudioReceiver::dataReceived,
                     processor, &DataProcessor::processRawData);

    if (ui->Osc_w) {
        QObject::connect(processor, &DataProcessor::waveformDataReady,
                         ui->Osc_w, &OscillogramWidget::appendSamples);
    } else {
        qWarning() << "Nie znaleziono widgetu Oscillogram w MainWindow UI";
    }
    
    if (ui->Spekt_w) { // TUTAJ Spekt_w JEST TYLKO NA CHWILĘ TO JEST TAK NAPRAWDĘ FFT
        QObject::connect(processor, &DataProcessor::FFTDataReady,
                         ui->Spekt_w, &FFTWidget::updateSpectrum);
    } else {
        qWarning() << "Nie znaleziono widgetu FFT (fft) w MainWindow UI";
    }

    // Obsługa błędów
    QObject::connect(receiver, &McuAudioReceiver::errorOccurred, [](const QString& errorMsg) {
        qCritical() << "!!! BŁĄD KOMUNIKACJI:" << errorMsg;
    });

    connect(thread, &QThread::started, receiver, &McuAudioReceiver::start);

    thread->start();
}

MainWindow::~MainWindow()
{
    thread->quit();
    if(!thread->wait(3000)) {
        thread->terminate();
    }
    delete ui;
}

void MainWindow::on_FFT_b_clicked()
{
    ui->Widoki_an_sw->setCurrentWidget(ui->FFT_w);
}

void MainWindow::on_Osc_Spekt_b_clicked()
{
    ui->Widoki_an_sw->setCurrentWidget(ui->Osc_Spekt_w);
}


void MainWindow::on_Powrot_an_b_clicked()
{
    ui->Widoki_sw->setCurrentWidget(ui->MainMenu_w);
}

void MainWindow::on_Powrot_muz_b_clicked()
{
    ui->Widoki_sw->setCurrentWidget(ui->MainMenu_w);
}

void MainWindow::on_WidokAn_b_clicked()
{
    ui->Widoki_sw->setCurrentWidget(ui->WidokAnalityczny_w);
}

void MainWindow::on_WidokMuz_b_clicked()
{
    ui->Widoki_sw->setCurrentWidget(ui->WidokMuzyczny_w);
}

void MainWindow::on_Opcje_b_clicked()
{
    ui->Widoki_sw->setCurrentWidget(ui->Opcje_w);
}

void MainWindow::on_Wyjscie_b_clicked()
{
    this->close();
}