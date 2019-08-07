#pragma once

#include <QSlider>

class Slider : public QSlider
{
    Q_OBJECT

public:

    Slider(QWidget *parent = nullptr);
    Slider(Qt::Orientation orientation, QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *e) override;
//    void mouseReleaseEvent(QMouseEvent *e) override;
signals:
    void newSelectedSliderPos(int position);
};


