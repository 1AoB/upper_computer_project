#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include "sendpack.h"
#include "receivepack.h"

#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QLabel>
#include <QPropertyAnimation>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void monitor();

    void updateData(char cmd,char param,int data);




private slots:
    void on_pushButton_add_clicked();

    void on_pushButton_sub_clicked();

    void setNeedleValue(double value);

private:
    Ui::MainWindow *ui;
    QSerialPort * serialPort;//串口
    SendPack *senderThread;//串口发送线程
    ReceivePack * receiveThread;//串口接收线程


    QGraphicsProxyWidget* m_needle;
    QPropertyAnimation *animation;
    //int degree;
};
#endif // MAINWINDOW_H
