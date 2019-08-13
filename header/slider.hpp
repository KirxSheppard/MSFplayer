#pragma once

#include <QSlider>

class Slider : public QSlider
{
    Q_OBJECT

public:
    Slider(QWidget *parent = 0);
    ~Slider();

    void setMyValue(int val);

private:
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;

//signals:
//    void userValueChanged(int value);

private:
    bool m_canUpdate = true;
    int m_pendingVal = ~0;
};
