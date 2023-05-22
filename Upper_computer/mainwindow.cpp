#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "protocal.h"
#include <QDateTime>
#include <QDebug>

#include <QGraphicsView>
#include <QMessageBox>
#include <QMessageBox>
#include <QPainter>
#include <QTimer>

#include "needle.h"


#include <QPropertyAnimation>

static bool state[6] = {true};//6个压力 正常

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /****************实时时间******************************/
    // 创建一个QTimer对象，并将其连接到一个槽函数中
    QTimer *timer = new QTimer(this);

    //先将时间显示在label_time上
    QString currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    ui->label_time->setText(currentDateTime);

    connect(timer, &QTimer::timeout,this, [=](){
        QString currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        ui->label_time->setText(currentDateTime);
    });

    // 启动QTimer对象，以定时更新QLabel控件中的时间
    timer->start(1000); // 更新


    /*********************仪表盘***************************/
    ui->pushButton_add->setStyleSheet(
                "QPushButton{"
                "background-image:url(:/res/addn.png);"
                "padding:0;"
                "border:0;"
                "background-position:Center;}"
                "QPushButton:pressed{"
                "background-image:url(:/res/red.png);}"
                );
    ui->pushButton_sub->setStyleSheet(
                "QPushButton{"
                "background-image:url(:/res/redn.png);"
                "padding:0;"
                "border:0;"
                "background-position:Center;}"
                "QPushButton:pressed{"
                "background-image:url(:/res/green.png);}"
                );

    //(1060,520 10x75)
    //(1060+75,520+10)
    //怎么让一个QLabel围绕一个点转
    //创建一个图像场景
    QGraphicsScene* scene = new QGraphicsScene();//首先,创建一个QGraphicsScene对象,这个对象就是一个可视化的场景,可以在其中添加各种图形元素。
    //设置场景的位置区域
    scene->setSceneRect(ui->graphicsView->rect());//接着，通过调用setSceneRect()方法设置了场景的位置区域，这里使用了ui->graphicsView->rect()，也就是代码中graphicsView对象的矩形区域，这个矩形区域通常是一个矩形窗口。

    //把指针窗口添加到场景中, 并返回用来操控指针窗口的“代理”m_needle
    m_needle = scene->addWidget(new Needle);//然后，代码创建了一个指针窗口，通过addWidget()方法将其添加到场景中，并返回一个QGraphicsProxyWidget对象，这个对象可以用来操控指针窗口。
    //指针窗口实际上是一个QWidget对象，可以自定义其外观和行为。在这里，指针窗口是通过调用一个自定义的Needle类来创建的。

    //设置指针窗口在场景中的位置
    m_needle->setPos(100,  103);//接下来，代码设置了指针窗口在场景中的位置，通过调用setPos()方法设置了指针窗口的位置，这里设置为(100, 103)，也就是在场景中的坐标系中的位置。
    //旋转原点在指针图片内部的(4,0)位置
    m_needle->setTransformOriginPoint(4,0); //然后，通过调用setTransformOriginPoint()方法设置了指针窗口的旋转原点，也就是在指针图片内部的(4, 0)位置。这个旋转原点会影响指针窗口的旋转行为，使得指针能够围绕这个点旋转。
    //这个(4,0)是:/res/needle.png中的位置

    //设置图像视图显示的场景
    ui->graphicsView->setScene(scene);//代码将场景设置为ui->graphicsView对象的场景，也就是将场景添加到了一个图像视图中，可以在这个视图中显示场景中的图形元素。这里使用了ui->graphicsView->setScene()方法来设置场景。


    // 创建旋转动画
    /*animation = new QPropertyAnimation(m_needle, "rotation");
    animation->setDuration(1000); // 设置动画持续时间为1秒
    animation->setStartValue(0); // 规定动画起始位置为0度
    animation->setEndValue(-130); // 要从0度,转多少度
    animation->setEasingCurve(QEasingCurve::InOutQuad); // 设置动画缓动曲线
    animation->setLoopCount(1); // 将循环次数设置为为1次

    // 启动旋转动画
    animation->start();

    degree = 0-130;*/


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::monitor()
{
    //串口初始化
    serialPort = new QSerialPort;
    //设置使用哪个串口
    serialPort->setPortName("COM2");
    //打开串口
    if(!serialPort->open(QIODevice::ReadWrite)){
        QMessageBox::critical(nullptr,
                              "串口打开失败",
                              "串口COM2不存在或者已被占用!",
                              QMessageBox::Ok
                              );
        //串口都打不开就直接关闭整个程序
        QTimer::singleShot(0,qApp,SLOT(quit()));
    }
    serialPort->setBaudRate(19200);//设置波特率
    serialPort->setDataBits(QSerialPort::Data8);//设置数据位
    serialPort->setParity(QSerialPort::NoParity);//设置校验位
    serialPort->setStopBits(QSerialPort::OneStop);//停止位
    serialPort->setFlowControl(QSerialPort::NoFlowControl);//设置流控,没有流控

    //启动发送线程
    senderThread = new SendPack(serialPort);
    senderThread->start();//在调用QThread对象的start()方法后，线程将开始运行，并自动调用run()方法。

    receiveThread = new ReceivePack(serialPort);
    receiveThread->start();

    //在线程创建之后,关联:接收线程发信号
    connect(receiveThread,
            &ReceivePack::receiveResponsePack,
            this,
            &MainWindow::updateData);


    //手动发送加速按钮的单击信号
    emit ui->pushButton_add->clicked();

}

void MainWindow::updateData(char cmd, char param, int data)
{
    //to do...
    //QMessageBox::information(this,"Tips","进入updateData函数!");
    switch (cmd)
    {
        case ZHU_JI_WEN_DU_RSP://RSP表示响应命令!!!
            ui->label_wen_du->setText(QString::number(data));
            break;
        case ZHOU_SU_DU_RSP:
        {
            QLabel *labels[2] = {ui->label_zhu_zhou, ui->label_fu_zhou};//下标:0 1
            if (param != 1 && param != 2)break;//param是参数,从1开始,1 2
            labels[param-1]->setText(QString::number(data));
            break;
        }
        case GANG_YOU_LIANG_RSP://缸油量
        {
            QLabel *labels[] =
            {
                ui->label_gang_1,
                ui->label_gang_2,
                ui->label_gang_3
            };//下标分别是:0 1 2
            if(param <1 || param > 3)break;//param是参数,从1开始,1 2 3
            labels[param-1]->setText(QString::number(data)+" L");//多少升油量
            break;
        }
        case YA_LI_CHANG_RSP://压力仓压力
        {
            QWidget * widgets[5][2] = {//进度条和文本
                                       {ui->progressBar_chang_1, ui->label_chang_1},
                                       {ui->progressBar_chang_2, ui->label_chang_2},
                                       {ui->progressBar_chang_3, ui->label_chang_3},
                                       {ui->progressBar_chang_4, ui->label_chang_4},
                                       {ui->progressBar_chang_5, ui->label_chang_5},
                                      };
            if(param < 1 || param > 5)break;
            if(data>1200)data=1200;//最大我们就设置为1200
            QProgressBar *bar = (QProgressBar*)widgets[param-1][0];//进度条
            bar->setValue(data);
            QLabel *label = (QLabel*)widgets[param-1][1];//文本
            label->setText(QString::number(data));
            //我们设置的范围是500~1200
            //当>1150时,改变进度条的颜色为红色,并改变label_state的图片为  :/res/warning.png
            if(data>1150){//报警
                //红色
                state[param-1] = false;
                bar->setStyleSheet("QProgressBar::chunk { background-color: red; }");
            }else{
                //绿色    :/res/normal.png
                bar->setStyleSheet("QProgressBar::chunk {background-color:rgb(0,185,84);}");
                state[param-1] = true;
            }
            bool tmp = state[0] && state[1] && state[2] && state[3] && state[4] && state[5];
            if(!tmp){//6个测压力的,只要有一个不在范围就报警
                QPixmap pixmap(":/res/warning.png");
                ui->label_state->setPixmap(pixmap);
            }else{
                QPixmap pixmap(":/res/normal.png");
                ui->label_state->setPixmap(pixmap);
            }
            break;
        }
        case JI_XIE_BI_RSP://只有一个
        {
            if(data>1200)data=1200;//最大我们就设置为1200
            ui->progressBar_ji_xie_bi->setValue(data);
            ui->label_ji_xie_bi->setText(QString::number(data));
            if(data>1150){//报警
                //红色
                state[6-1] = false;
                ui->progressBar_ji_xie_bi->setStyleSheet("QProgressBar::chunk { background-color: red; }");
            }else{
                //绿色    :/res/normal.png
                ui->progressBar_ji_xie_bi->setStyleSheet("QProgressBar::chunk {background-color:rgb(0,185,84);}");
                state[6-1] = true;
            }
            bool tmp = state[0] && state[1] && state[2] && state[3] && state[4] && state[5];
            if(!tmp){//6个测压力的,只要有一个不在范围就报警
                QPixmap pixmap(":/res/warning.png");
                ui->label_state->setPixmap(pixmap);
            }else{
                QPixmap pixmap(":/res/normal.png");
                ui->label_state->setPixmap(pixmap);
            }
            break;
        }
        case DONG_LI_GAN_SU_DU_RSP://动力杆速度响应命令
        {
            //QMessageBox::information(this,"Tips","来喽!");
            setNeedleValue(data);
            break;
        }
    }


}


void MainWindow::on_pushButton_add_clicked()
{
    //仪表60是0 度,0是-130度,120是130度
    //所以degree是[-130,130]
    /*qDebug()<<"加速:";
    animation->setDuration(500); // 设置动画持续时间为0.5秒
    animation->setStartValue(degree); // 规定动画起始位置
    degree += 20;
    if(degree<-130 || degree>130){
        degree -= 20;
        return;
    }
    animation->setEndValue(degree); // 转多少度
    animation->setEasingCurve(QEasingCurve::InOutQuad); // 设置动画缓动曲线
    animation->setLoopCount(1); // 将循环次数设置为1次

    // 启动旋转动画
    animation->start();*/
    char pack[9] = {0,};
    senderThread->makePack(DONG_LI_GAN_SU_DU_REQ, 0xF0, 0, pack);
    int ret = serialPort->write(pack, sizeof(pack));
    qDebug() << "send: " << ret << " bytes.";

}

void MainWindow::on_pushButton_sub_clicked()
{
    /*qDebug()<<"减速";
    animation->setDuration(500); // 设置动画持续时间为0.5秒
    animation->setStartValue(degree); // 规定动画起始位置
    degree -= 20;
    if(degree<-130 || degree>130){
        degree += 20;
        return;
    }
    animation->setEndValue(degree); // 转多少度
    animation->setEasingCurve(QEasingCurve::InOutQuad); // 设置动画缓动曲线
    animation->setLoopCount(1); // 将循环次数设置为1次

    // 启动旋转动画
    animation->start();*/
    char pack[9] = {0,};
    senderThread->makePack(DONG_LI_GAN_SU_DU_REQ, 0x0F, 0, pack);
    int ret = serialPort->write(pack, sizeof(pack));//我向下位机发起查询命令,下位机回给我响应命令
    qDebug() << "send: " << ret << " bytes.";
}

void MainWindow::setNeedleValue(double value)   //码表值：0-120   度数范围：54-310(手动测出来的)
{
    if(value < 0) value = 0;
    else if(value > 120) value = 120;

    double rotateAngle = (double)((310.0 - 54.0) / 120) * value + 54.00;//(初始值+一份)

    //微调
    if(value < 40) rotateAngle += 1;
    else if(value > 40) rotateAngle -= 2;
    /*
     * 微调是为了让指针显示更加准确。在这里，如果value的值小于40，就向旋转角度中加1度；如果value的值大于40，就向旋转角度中减2度。
     * 这些微调是通过实验得出的，使得指针在指向不同的区间时，能够更加精准地显示数据。
    */

    m_needle->setRotation(rotateAngle);
    //setRotation是一个系统函数
    /*
     * 这段代码是用来设置指针窗口旋转角度的。其中，m_needle是一个QGraphicsProxyWidget对象，
     * 代表了指针窗口，setRotation()是QGraphicsItem类中的一个成员函数，用于设置图形项的旋转角度。
    具体来说，setRotation()函数接受一个旋转角度参数，单位为度数，表示图形项将会被旋转多少度。

    例如，如果将旋转角度设置为90，那么图形项将会顺时针旋转90度。
    在这段代码中，rotateAngle是一个变量，表示了指针需要旋转的角度。
    这个变量的值可以通过一些计算或者从其他地方获取。
    然后，调用setRotation()函数，将rotateAngle作为参数传入，就可以将指针窗口旋转到相应的角度。
    通过不断更新旋转角度，可以使指针窗口指向不同的位置，从而实现类似仪表盘的效果。
    */
}




