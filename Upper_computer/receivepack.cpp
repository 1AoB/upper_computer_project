#include "receivepack.h"

#include <QDebug>

ReceivePack::ReceivePack(QSerialPort * port)
{
    serialPort = port;
}

//qt线程类的run方法
void ReceivePack::run()
{
    unsigned char pack[9];

    //下面三个是传入传出参数
    char cmd;
    char param;
    int data;

    while(1){
        serialPort->waitForReadyRead(100);//等待有数据可读,没有就等100毫秒
        int ret = serialPort->bytesAvailable();//有多少个可用的字节
        if(ret == 0)continue;

        //能走到这,说明真正有数据可读
        ret = serialPort->read((char*)pack,sizeof(pack));//接收
        qDebug()<<"read:"<<ret<<"bytes :"
               << pack [0]<< " "<<pack[1]<<" "<<pack[2]<<" "
               << pack [3]<< " "<<pack[4]<<" "<<pack[5]<<" "
               << pack [6]<< " "<<pack[7]<<" "<<pack[8]<<" ";
        if(parsePack(pack,sizeof(pack) , &cmd , &param, &data)){
            qDebug()<<"parse succeed!";
            emit receiveResponsePack(cmd, param, data);
        }else{
            qDebug()<<"收到非法包!";
        }
    }
}

//解析包
/**
  * 数据源
  * 数据源大小
  * 命令
  * 参数
  * 返回数据
***/
bool ReceivePack::parsePack(unsigned char *pack, int size, char *cmd,
                            char *param, int *data)
{
    if(!pack || size < 9)return false;

    //包头和包尾是否合法
    if(pack[0] == 0xEF && pack[8]==0xFE){
        unsigned char *p = pack + 1 ;//我们要的是包里面,下标1~7的数据
        //判断1的个数
        int count = 0;
        for(int i = 0; i < 7 ;i++,p++){//命令+校验位一共七个字节
            char tmp = *p;
            for(int k = 0; k < 8 ; k++){//一个字节8位
                if(tmp & 1){count++;}
                tmp >>= 1;
            }
        }
        if(count % 2){
            return false;//不满足奇偶校验的规则
        }
        *cmd = pack[1];
        *param = pack[2];
        //注意大小端,数据位的下标是3 4 5 6
        //原来的排列顺序是:3 4 5 6
        //现在的排列顺序是:6 5 4 3
        *data = pack[3] | (pack[4]<<8) | (pack[5]<<16) | (pack[6]<<24);//pack[6]是奇偶校验位
        qDebug()<< "data : " << *data ;
        return true;
    }
    return false;
}
