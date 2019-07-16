#include "mainwindow.h"
#include <QtGui>

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
        painter.drawPixmap(0, 0, QPixmap("C:/Users/Sheppard/Desktop/1.bmp"));

}
