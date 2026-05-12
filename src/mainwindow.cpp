#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "OscillogramWidget.h"
#include "FFTWidget.h"
#include "SpectrogramWidget.h"
#include "style.h"

#include <iostream>

MainWindow::MainWindow(QWidget *parent):
     QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // --- NAWIGACJA: MENU GŁÓWNE ---
    connect(ui->WidokAn_b, &QPushButton::clicked, [this]() { ui->Widoki_sw->setCurrentWidget(ui->WidokAnalityczny_w); emit(needToCalc(Needed::Analytics));});
    connect(ui->WidokMuz_b, &QPushButton::clicked, [this]() { ui->Widoki_sw->setCurrentWidget(ui->WidokMuzyczny_w); emit(needToCalc(Needed::Musical));});
    connect(ui->Wyjscie_b,  &QPushButton::clicked, this, &QMainWindow::close);

    // --- NAWIGACJA: WIDOK ANALITYCZNY ---
    connect(ui->FFT_b,       &QPushButton::clicked, [this]() { ui->Widoki_an_sw->setCurrentWidget(ui->FFT_w); });
    connect(ui->Osc_Spekt_b, &QPushButton::clicked, [this]() { ui->Widoki_an_sw->setCurrentWidget(ui->Osc_Spekt_w); });
    connect(ui->Powrot_an_b, &QPushButton::clicked, [this]() { ui->Widoki_sw->setCurrentWidget(ui->MainMenu_w); });

    // --- NAWIGACJA: WIDOK MUZYCZNY ---
    connect(ui->Pianino_b,    &QPushButton::clicked, [this]() { ui->Widoki_muz_sw->setCurrentWidget(ui->Pianino_w); });
    connect(ui->Pieciolinia_b, &QPushButton::clicked, [this]() { ui->Widoki_muz_sw->setCurrentWidget(ui->Pieciolinia_w); });
    connect(ui->Powrot_muz_b,  &QPushButton::clicked, [this]() { ui->Widoki_sw->setCurrentWidget(ui->MainMenu_w); });


    this->setStyleSheet(myTheme);

    back_option = FromWhere::MainMenu;

    ui->Widoki_sw->setCurrentWidget(ui->MainMenu_w);
    ui->Widoki_an_sw->setCurrentWidget(ui->Osc_Spekt_w);
    ui->Widoki_muz_sw->setCurrentWidget(ui->Pianino_w);

    thread = new QThread(this);

    receiver = new McuAudioReceiver();
    processor = new DataProcessor();
    
    connect(this, &MainWindow::needToCalc, processor, &DataProcessor::whatsNeeded);

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
    
    // Podłączenie spektrogramu (widmo FFT -> kolumna spektrogramu)
    if (ui->Spekt_w) {
        QObject::connect(processor, &DataProcessor::FFTDataReady,
                         ui->Spekt_w, &SpectrogramWidget::appendSpectrum);
    } else {
        qWarning() << "Nie znaleziono widgetu Spektrogramu w MainWindow UI";
    }

    // Podłączenie widgetu FFT (widmo FFT -> słupki FFT)
    if (ui->FFT_w) {
        QObject::connect(processor, &DataProcessor::FFTDataReady,
                         ui->FFT_w, &FFTWidget::updateSpectrum);
    } else {
        qWarning() << "Nie znaleziono widgetu FFT w MainWindow UI";
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

// Przyciski od Opcji

void MainWindow::on_Opcje_an_b_clicked() {
    ui->Widoki_sw->setCurrentWidget(ui->Opcje_w);
    emit(needToCalc(Needed::Idle));
    back_option = FromWhere::Analytics;
}

void MainWindow::on_Opcje_muz_b_clicked() {
    ui->Widoki_sw->setCurrentWidget(ui->Opcje_w);
    emit(needToCalc(Needed::Idle));
    back_option = FromWhere::Musical;
}

void MainWindow::on_Opcje_b_clicked() {
    ui->Widoki_sw->setCurrentWidget(ui->Opcje_w);
    emit(needToCalc(Needed::Idle));
    back_option = FromWhere::MainMenu;
}

void MainWindow::on_Powrot_opcje_b_clicked() {
    switch (back_option)
    {
    case FromWhere::MainMenu:
        ui->Widoki_sw->setCurrentWidget(ui->MainMenu_w);
        emit(needToCalc(Needed::Idle));
        break;
    case FromWhere::Analytics:
        ui->Widoki_sw->setCurrentWidget(ui->WidokAnalityczny_w);
        emit(needToCalc(Needed::Analytics));
        break;
    case FromWhere::Musical:
        ui->Widoki_sw->setCurrentWidget(ui->WidokMuzyczny_w);
        emit(needToCalc(Needed::Musical));
        break;
    default:
        break;
    }
}

// --- Kontrolki w Opcjach (Slidery i SpinBoxy) ---

// FFT min freq

void MainWindow::on_FFT_minfreq_s_valueChanged(int value) {
    if(ui->FFT_maxfreq_s->value() > value) {
        ui->FFT_minfreq_sb->setValue(value);
        ui->FFT_w->updateMinFreq(value);
    } else {
        ui->FFT_minfreq_s->setValue(ui->FFT_minfreq_sb->value());
    }
}

void MainWindow::on_FFT_minfreq_sb_valueChanged(int arg1) {
    if(ui->FFT_maxfreq_s->value() > arg1) {
        ui->FFT_minfreq_s->setValue(arg1);
        ui->FFT_w->updateMinFreq(arg1);
    } else {
        ui->FFT_minfreq_sb->setValue(ui->FFT_minfreq_s->value());
    }
}

// FFT max freq

void MainWindow::on_FFT_maxfreq_s_valueChanged(int value) {
    if(ui->FFT_minfreq_s->value() < value) {
        ui->FFT_maxfreq_sb->setValue(value);
        ui->FFT_w->updateMaxFreq(value);
    } else {
        ui->FFT_maxfreq_s->setValue(ui->FFT_maxfreq_sb->value());
    }
}

void MainWindow::on_FFT_maxfreq_sb_valueChanged(int arg1) {
    if(ui->FFT_minfreq_s->value() < arg1) {
        ui->FFT_maxfreq_s->setValue(arg1);
        ui->FFT_w->updateMaxFreq(arg1);
    } else {
        ui->FFT_maxfreq_sb->setValue(ui->FFT_maxfreq_s->value());
    }
}

// Oscyloskop Time

void MainWindow::on_Osc_Spekt_time_s_valueChanged(int value) {
    ui->Osc_Spekt_time_dsb->setValue(static_cast<double>(value)/10.0);
    if (ui->Osc_w) ui->Osc_w->updateTime(static_cast<double>(value)/10.0);
}

void MainWindow::on_Osc_Spekt_time_dsb_valueChanged(double arg1) {
    ui->Osc_Spekt_time_s->setValue(static_cast<int>(arg1*10));
    if (ui->Osc_w) ui->Osc_w->updateTime(arg1);
}

// Oscyloskop Gain
void MainWindow::on_Osc_gain_s_valueChanged(int value) {
    ui->Osc_gain_dsb->setValue(static_cast<double>(value)/10.0);
    if (ui->Osc_w) ui->Osc_w->updateGain(static_cast<double>(value)/10.0);
}

void MainWindow::on_Osc_gain_dsb_valueChanged(double arg1) {
    ui->Osc_gain_s->setValue(static_cast<int>(arg1*10));
    if (ui->Osc_w) ui->Osc_w->updateGain(arg1);
}