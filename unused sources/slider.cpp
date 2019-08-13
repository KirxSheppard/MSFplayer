#include "slider.hpp"
#include <qevent.h>
#include <QDebug>

Slider::Slider(QWidget *parent)
    : QSlider(parent)
{

}

Slider::Slider(Qt::Orientation orientation, QWidget *parent)
    : QSlider(orientation, parent)
{

}

void Slider::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        QMouseEvent ev(e->type(), e->pos(), Qt::MidButton, Qt::MidButton, e->modifiers());
        QSlider::mousePressEvent(&ev);
        int pos = value();
//        qDebug()<<"pressed slider class: "<<pos;
        emit newSelectedSliderPos(pos);
    }
    else
    {
        QSlider::mousePressEvent(e);
    }
}
