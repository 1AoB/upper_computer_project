#ifndef RECEIVEPACK_H
#define RECEIVEPACK_H

#include <QObject>
#include <QSerialPort>
#include <QThread>

//这个类是接收数据包的线程类


class ReceivePack : public QThread
{
    Q_OBJECT
public:
    ReceivePack(QSerialPort * port);
    void run() override;

signals:
    void receiveResponsePack(char cmd, char param, int data);

private:
    QSerialPort * serialPort;//用来指向当前的串口

    //解析包             数据源
    bool parsePack(unsigned char *pack , int size ,char* cmd,
                   char* param, int* data);
};

#endif // RECEIVEPACK_H
