#pragma once

#include <QMainWindow>

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

private slots:

    // przyciski widoku analitycznego

    void on_fft_b_clicked();
    void on_osc_spektr_b_clicked();
    void on_powrot_an_b_clicked();

    // przyciski widoku muzycznego

    void on_powrot_muz_b_clicked();

    // Przyciski menu głównego

    void on_WidokAn_b_clicked();
    void on_WidokMuz_b_clicked();
    void on_Opcje_b_clicked();
    void on_Wyjscie_b_clicked();
};
