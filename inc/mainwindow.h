#pragma once

#include <QMainWindow>
#include <QThread>
#include "McuAudioReceiver.h"
#include "DataProcessor.h"

enum class FromWhere {
    MainMenu,
    Analytics,
    Musical
};


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {

    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QThread* thread;
    McuAudioReceiver* receiver;
    DataProcessor* processor;

    FromWhere back_option;


private slots:
    // Przyciski od Opcji
    void on_Opcje_an_b_clicked();
    void on_Opcje_muz_b_clicked();
    void on_Opcje_b_clicked();

    void on_Powrot_opcje_b_clicked();

    // --- Kontrolki w Opcjach (Slidery i SpinBoxy) ---
    // FFT min freq
    void on_FFT_minfreq_s_valueChanged(int value);
    void on_FFT_minfreq_sb_valueChanged(int arg1);

    // FFT max freq
    void on_FFT_maxfreq_s_valueChanged(int value);
    void on_FFT_maxfreq_sb_valueChanged(int arg1);

    // Oscyloskop Time
    void on_Osc_Spekt_time_s_valueChanged(int value);
    void on_Osc_Spekt_time_dsb_valueChanged(double arg1);

    // Oscyloskop Gain
    void on_Osc_gain_s_valueChanged(int value);
    void on_Osc_gain_dsb_valueChanged(double arg1);

signals:
    void needToCalc(const Needed& needed);
};
