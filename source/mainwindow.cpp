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

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPixmap img("C:/Users/Sheppard/Desktop/test/TE000000.tiff");
    QPixmap img2 = img.scaled(960, 540, KeepAspectRatio);
        painter.drawPixmap(0, 0, QPixmap(img2));

}
