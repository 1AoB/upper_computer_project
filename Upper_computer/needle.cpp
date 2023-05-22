#include "needle.h"
#include <QPainter>

Needle::Needle(QWidget *parent) : QWidget(parent)
{
    //设置窗口大小
    setFixedSize(8, 63);

    //设置背景透明样式
    setStyleSheet("background:transparent;");
}

void Needle::paintEvent(QPaintEvent *event)
{
    //创建画刷
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);//抗锯齿

    //导入图片
    QPixmap pix(":/res/needle.png");

    //绘制图片
    painter.drawPixmap(this->rect(), pix);
}
