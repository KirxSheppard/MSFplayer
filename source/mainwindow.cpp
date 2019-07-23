#include "mainwindow.h"
#include <QtGui>
#include "ui_mainwindow.h"
#include <QMessageBox>


//using namespace Qt;

MainWindow::MainWindow(QString fileName, int numofFrames, QWidget *parent) :
    QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    numOfFrames = numofFrames;
    ui->setupUi(this);
    ui->statusbar->setStyleSheet("color: white");

    ui->statusbar->showMessage(fileName);
    ifExit = false;
    ifSave = false;
//    lookedNum = -1;
//    ui->horizontalSlider->QStatusBar::addPermanentWidget();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setImage(const QImage &img)
{
    m_img = img;
    update();
}

void MainWindow::setSliderValue(int numofFrames,int valueS)
{
    valueS = 100 / numofFrames * valueS;
    ui->horizontalSlider->setValue(valueS);
}

int MainWindow::setFrame()
{
    return numOfFrames;
}

bool MainWindow::checkifExit()
{
    return ifExit;
}

bool MainWindow::checkifSave()
{
    return ifSave;
}



void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawImage(rect(), m_img);
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
        numOfFrames = numOfFrames * value / 100;
//      lookedNum = num/value;
//    value = std::clamp(value,0, 100);
//    ui->horizontalSlider->setValue(value);
}

void MainWindow::on_actionexit_triggered()
{
    qDebug()<<"exit";
    ifExit = true;
//    QApplication::quit();
}

void MainWindow::on_actionSave_triggered()
{
    QMessageBox::information(this,"MSF", "Starting to save frames!");
    ifSave = true;
}
