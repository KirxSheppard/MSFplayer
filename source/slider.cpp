#include "slider.hpp"
#include <qevent.h>

Slider::Slider(QWidget *parent)
    : QSlider(parent)
{
//    connect(this, &Slider::sliderMoved,
//            this, [=](int val) {
//        emit userValueChanged(val);
//    });
//    connect(this, &Slider::sliderPressed,
//            this, [=] {
//        emit userValueChanged(value());
//    });
}
Slider::~Slider()
{
}

void Slider::setMyValue(int val)
{
    if (m_canUpdate)
        setValue(val);
    else
        m_pendingVal = val;
}

void Slider::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton || ev->button() == Qt::MiddleButton)
        m_canUpdate = false;

    if (ev->buttons() == Qt::LeftButton)
    {
        QMouseEvent e(
            ev->type(),
            ev->localPos(),
            ev->windowPos(),
            ev->screenPos(),
            Qt::MiddleButton,
            Qt::MiddleButton,
            ev->modifiers(),
            Qt::MouseEventSynthesizedByApplication
        );
        QSlider::mousePressEvent(&e);
        return;
    }

    QSlider::mousePressEvent(ev);
}
void Slider::mouseReleaseEvent(QMouseEvent *ev)
{
    QSlider::mouseReleaseEvent(ev);
    if (m_pendingVal != ~0)
    {
        setValue(m_pendingVal);
        m_pendingVal = ~0;
    }
    m_canUpdate = true;
}
