#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("Qt上位机");

    w.show();
    //启动监控
    w.monitor();


    return a.exec();
}

