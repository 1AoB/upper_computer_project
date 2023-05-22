#include "sendpack.h"
#include "protocal.h"

#include <QDebug>

SendPack::SendPack(QSerialPort *port)
{
    serialPort = port;
}

void SendPack::run()
{
    //发包就是命令和参数
    //数据位在收包时才有

    static unsigned char cmdAndParam[][2] = {
        {ZHU_JI_WEN_DU_REQ,0},
        {ZHOU_SU_DU_REQ,1},//主轴
        {ZHOU_SU_DU_REQ,2},//副轴
        {GANG_YOU_LIANG_REQ,1},//工作缸剩余油量
        {GANG_YOU_LIANG_REQ,2},
        {GANG_YOU_LIANG_REQ,3},
        {YA_LI_CHANG_REQ,1},//压力仓压力查询
        {YA_LI_CHANG_REQ,2},
        {YA_LI_CHANG_REQ,3},
        {YA_LI_CHANG_REQ,4},
        {YA_LI_CHANG_REQ,5},
        {JI_XIE_BI_REQ,0},//机械臂压力,就一个,这里参数就写0了

        //to do...
    };

    char pack[9];
    //char data[4] = {0, 0, 0, 0};//char data[4];
    //memset(data,0,sizeof data);
    int cmdCount =
            sizeof(cmdAndParam)/sizeof(cmdAndParam[0]);

    //每个几秒发一个数据查询包
    while(1){
        for(int i = 0; i < cmdCount ; i++)
        {
            //组好包了
            makePack(cmdAndParam[i][0],
                    cmdAndParam[i][1],
                    0,//data,
                    pack);
            //发包
            int ret =  serialPort->write(pack,sizeof(pack));
            qDebug()<<"send: "<< ret<<"bytes";
            //This function blocks until at least one byte has been written to the serial port and the bytesWritten() signal has been emitted.
            serialPort->waitForBytesWritten(10);//毫秒
            QThread::msleep(10);
        }
        QThread::msleep(2000);
    }
}


//pack存放组包结果
bool SendPack::makePack(char cmd, char param, char data[], char *pack)
{
    if(!pack)return false;

    pack[0] = 0xEF;//包头
    pack[1] = cmd;//命令
    pack[2] = param;//参数

    //数据位
    pack[3] = 0;
    pack[4] = 0;
    pack[5] = 0;
    pack[6] = 0;


    /////////////////////////////这里不同////////////////////////////////////////////
    //两种写法都可以,第一种写法遍历了命令(1字节),参数(1字节),数据位(4字节);
    //而第二种,则只是遍历命令(1字节),参数(1字节),因为我们是要给下位机发数据,所以数据位都是0(我们要计算的是1的个数,0自然不用遍历)
    //设置偶校验(与具体的下位机保持一致)
    int count = 0;
    for(int k = 1 ; k <= 6 ; k ++){//6个字节
        char byte = pack[k];
        for(int i = 0 ; i < 8 ; i ++){//每个字节8位
            if(byte & 1){
                count ++;
            }
            byte >>= 1;
        }
    }
    /*for(int i = 0; i < 8 ; i++){
        if(cmd & 1)count++;
        cmd >>= 1;
    }
    for(int i = 0; i < 8 ; i++){
        if(param & 1)count++;
        param >>= 1;
    }*/


    if(count & 1){//偶校验
        //奇数
        pack[7] = 1;
    }else{
        //偶数
        pack[7] = 0;
    }

    pack[8] = 0xFE;//包尾
    return true;
}
