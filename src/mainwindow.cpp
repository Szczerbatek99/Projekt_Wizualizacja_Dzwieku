#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent):
     QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->Widoki->setCurrentWidget(ui->MainMenu);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_fft_b_clicked()
{
    ui->widoki_an->setCurrentWidget(ui->fft_w);
}

void MainWindow::on_osc_spektr_b_clicked()
{
    ui->widoki_an->setCurrentWidget(ui->osc_spekt_w);
}


void MainWindow::on_powrot_an_b_clicked()
{
    ui->Widoki->setCurrentWidget(ui->MainMenu);
}

void MainWindow::on_powrot_muz_b_clicked()
{
    ui->Widoki->setCurrentWidget(ui->MainMenu);
}


void MainWindow::on_WidokAn_b_clicked()
{
    ui->Widoki->setCurrentWidget(ui->WidokAnalityczny);
}

void MainWindow::on_WidokMuz_b_clicked()
{
    ui->Widoki->setCurrentWidget(ui->WidokMuzyczny);
}

void MainWindow::on_Opcje_b_clicked()
{
    ui->Widoki->setCurrentWidget(ui->Opcje);
}

void MainWindow::on_Wyjscie_b_clicked()
{
    this->close();
}