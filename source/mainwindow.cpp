#include "mainwindow.h"
#include <QtGui>
#include "ui_mainwindow.h"
#include <QMessageBox>


//using namespace Qt;

MainWindow::MainWindow(QString fileName, int numofFrames, QWidget *parent) :
    QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    ui->statusbar->setStyleSheet("color: white");
    ui->statusbar->showMessage(fileName);
    ui->statusbar->addPermanentWidget(ui->progressBar);
    QString progressLook = "QProgressBar::chunk {background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0,stop: 0 #78d,stop: 0.2999 #46a,stop: 0.5 #45a,stop: 1 #238 );"
                  "border-bottom-right-radius: 0px;"
                  "border-bottom-left-radius: 0px;"
                  "border: 0px solid black;}";

    ui->progressBar->setStyleSheet(progressLook);

    ui->progressBar->hide();

    ifExit = false;
    ifSave = false;
    ifPaused = false;
    ifWaterMark = false;
    numOfFrames = numofFrames;
    frameSaveCounter = 0;

//         ui->playPauseButton->setStyleSheet(QString::fromUtf8("background-image: url(:/resources/play.png);"));
         ui->playPauseButton->setStyleSheet(QString::fromUtf8("background-color: argb(255,0,0,0);"));
         ui->playPauseButton->setIcon(QIcon(":/resources/pause.png"));




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
    if(ifSave==true)
    {
        ui->progressBar->show();


        frameSaveCounter++;
        int temp = ((double) frameSaveCounter / numOfFrames) * 100;
        ui->progressBar->setValue(temp);

        if(frameSaveCounter == numOfFrames)
        {
            ifSave = false;
            ui->progressBar->hide();
            QMessageBox::information(this, "Status inspector", "All images saved successfully!");
        }
    }
    return ifSave;
}

bool MainWindow::checkifPaused()
{
    return ifPaused;
}

bool MainWindow::checkWaterMark()
{
    return ifWaterMark;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawImage(rect(), m_img);
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
//        numOfFrames = numOfFrames * value / 100;
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
    ifSave = true;
}

void MainWindow::on_playPauseButton_clicked()
{
//    qDebug()<<"checked: "<<checked;
    if(ifPaused == false)
    {
        ui->playPauseButton->setIcon(QIcon(":/resources/play.png"));
        ifPaused = true;
    }
    else
    {
        ui->playPauseButton->setIcon(QIcon(":/resources/pause.png"));
        ifPaused = false;
    }

}

void MainWindow::on_actionAbout_this_app_triggered()
{
     QMessageBox::about(this, "About this app", "This is first MSF's GUI app fully written in C++. This program demuxes and decodes the given video and allows to add watermark, change brightness and save particular frames.");
}

void MainWindow::on_actionwater_mark_triggered()
{
    ifWaterMark ? ifWaterMark = false : ifWaterMark = true;
}
