#pragma once

#include <QMainWindow>
#include <QThread>
#include "McuAudioReceiver.h"
#include "DataProcessor.h"

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

private slots:

    // przyciski widoku analitycznego

    void on_FFT_b_clicked();
    void on_Osc_Spekt_b_clicked();
    void on_Powrot_an_b_clicked();

    // przyciski widoku muzycznego

    void on_Powrot_muz_b_clicked();

    // Przyciski menu głównego

    void on_WidokAn_b_clicked();
    void on_WidokMuz_b_clicked();
    void on_Opcje_b_clicked();
    void on_Wyjscie_b_clicked();
};
