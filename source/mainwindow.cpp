#include "mainwindow.h"
#include <QtGui>

using namespace Qt;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{

}

MainWindow::~MainWindow()
{
}

void MainWindow::setImage(const QImage &img)
{
    m_img = img;
    update();
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawImage(rect(), m_img);
}
