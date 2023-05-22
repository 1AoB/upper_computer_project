#ifndef SENDPACK_H
#define SENDPACK_H

#include <QThread>
#include <QSerialPort>

//这个是发送数据包的线程类

class SendPack : public QThread
{
    Q_OBJECT
public:
    SendPack(QSerialPort *port);

    void run() override;

    bool makePack(char cmd, char param,char data[],char *pack);
private:
    QSerialPort * serialPort;

};

#endif // SENDPACK_H
