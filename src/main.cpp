#include "mainwindow.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qDebug() << "Uruchamianie wizualizera dźwięku...";

    qRegisterMetaType<std::vector<int32_t>>("std::vector<int32_t>");
    qRegisterMetaType<std::vector<double>>("std::vector<double>");
    qRegisterMetaType<Needed>("Needed");

    MainWindow w;
    w.show();

    return a.exec();
}