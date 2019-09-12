#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <iostream>
#include <QFileDialog>
#include <cmath>
#include <QTimer>
#include <cctype>

constexpr int g_timeOut = 4000;

MainWindow::MainWindow(const QString msfLogo, QWidget *parent) :
    QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->widgetInspector->hide();
    timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->start(g_timeOut);

    vidWidget = ui->widgetVideo;

//    ui->statusbar->setStyleSheet("color: white");
//    ui->statusbar->showMessage(mFileNameWithFormat);
//    ui->statusbar->addPermanentWidget(ui->progressBar);
    ui->statusbar->hide();
    QString progressLook = "QProgressBar::chunk {background: QLinearGradient(x1: 0, y1: 0, x2: 0.5, y2: 0,stop: 0 #78d,stop: 0.2999 #46a,stop: 0.5 #45a,stop: 1 #006a80 );"
                  "border-bottom-right-radius: 0px;"
                  "border-bottom-left-radius: 0px;"
                  "border-top-right-radius: 0px;"
                  "border-top-left-radius: 0px;"
//                           "hight: 10px;"
                  "border: 0px solid black;}";

    ui->progressBar->setStyleSheet(progressLook);
    ui->progressBar->hide();
    ui->playPauseButton->setStyleSheet("background-color: transparent; border: none;");
    ui->playPauseButton->setIcon(QIcon(":/resources/pause.png"));
    ui->timelabel->setStyleSheet("color: white");
    ui->timelabel->setText(mVideoCurrPos);
    ui->labelRemainingTime->setStyleSheet("color: white");
    ui->labelRemainingTime->setText(mRemVideoTime);


    opacityEffect = new QGraphicsOpacityEffect;
    opacityEffect->setOpacity(1.0);
    ui->vidProgWidget->setGraphicsEffect(opacityEffect);

    animation = new QPropertyAnimation(opacityEffect, "opacity");
    animation->setEasingCurve(QEasingCurve::Type::OutCubic);
    animation->setStartValue(1.0);
    animation->setEndValue(0.0);
    animation->setDuration(750);

    connect(animation, &QPropertyAnimation::finished,
            this, [=] {
        ui->vidProgWidget->setVisible(false);
    });

    ifSave = false;
    ifPaused = false;
    ifNewSlierValue = false;

    mFrameSaveCounter = 0;
    mMsfLogoPath = msfLogo;
    mWaterMarkPath = mMsfLogoPath;
    vidWidget->msfWaterMarkPath(msfLogo);


    connect(timer, &QTimer::timeout, this, &MainWindow::hideInterface);

    connect(&decoder, &Decoder::videoTimeCode,
            this, &MainWindow::setVideoTimeCode);
    connect(&decoder, &Decoder::mRgb,
            vidWidget, &VideoWidget::setImage);
    connect(&decoder, &Decoder::positon,
            this, &MainWindow::setSliderValue);
    connect(ui->widgetInspector, &Inspector::ifBrightBool,
            &decoder, &Decoder::setBrightState);
    connect(ui->widgetInspector, &Inspector::brightValue,
            &decoder, &Decoder::setBrightness);
    connect(ui->widgetInspector, &Inspector::ifRedChannel,
            &decoder, &Decoder::setRedChannelState);
    connect(ui->widgetInspector, &Inspector::redValue,
            &decoder, &Decoder::setRedChannel);
    connect(ui->widgetInspector, &Inspector::ifGreenChannel,
            &decoder, &Decoder::setGreenChannelState);
    connect(ui->widgetInspector, &Inspector::greenValue,
            &decoder, &Decoder::setGreenChannel);
    connect(ui->widgetInspector, &Inspector::ifBlueChannel,
            &decoder, &Decoder::setBlueChannelState);
    connect(ui->widgetInspector, &Inspector::blueValue,
            &decoder, &Decoder::setBlueChannel);
    connect(ui->widgetInspector, &Inspector::wmOpacityValue,
            vidWidget, &VideoWidget::setWmOpacityValue);
    connect(ui->widgetInspector, &Inspector::wmPosX,
            vidWidget, &VideoWidget::setWmPosX);
    connect(ui->widgetInspector, &Inspector::wmPosY,
            vidWidget, &VideoWidget::setWmPosY);
    connect(ui->widgetInspector, &Inspector::wmSetPressed,
            vidWidget, &VideoWidget::setWmPath);
    connect(ui->widgetInspector, &Inspector::wmResetPath,
            vidWidget, &VideoWidget::resetWmPath);
    connect(ui->widgetInspector, &Inspector::wmScaleValue,
            vidWidget, &VideoWidget::setWmScaleValue);
    connect(vidWidget, &VideoWidget::clearWmPath,
            ui->widgetInspector, &Inspector::resetWm2BigPic);
    connect(vidWidget, &VideoWidget::afterUpdateIfPaused,
            this, &MainWindow::afterUpdateFrameIfPaused);
    connect(vidWidget, &VideoWidget::checkIfSaving,
            this, &MainWindow::checkifSave);
}

MainWindow::~MainWindow()
{
    decoder.stop();
    decoder.wait();
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    const QSize imgS =  m_imgGOps.size();
    QSize s = imgS.scaled(size(), Qt::KeepAspectRatio);
//    qDebug()<<"window size: "<<s;

    //background
    p.fillRect(this->rect(), QColor(mBgColor));

    p.drawImage(QRect(QPoint((width() - s.width()) / 2, 0), s), m_imgGOps);

    //On screen rect for play/pause button
    onScreenPlayPause = QRect(s.width() / 4,
                              s.height() / 4,
                              s.width() / 2,
                              s.height() / 2);

    //here it works fine without any weird glitches except Windows (not anymore, needs some work)
#ifndef Q_OS_WINDOWS
//    this->resize(s.width(),s.height());
#endif

    if(auto fps = decoder.getVideoFps(); fps == fps)
    {
//        ui->statusbar->showMessage(mFileNameWithFormat + " (" + QString::number(decoder.getVideoFps()) + "fps)");
        setWindowTitle(mFileNameWithFormat + " (" + QString::number(decoder.getVideoFps()) + "fps)");
    }
    else {
        ui->vidProgWidget->hide();
//        ui->statusbar->showMessage(mFileNameWithFormat);
        setWindowTitle(mFileNameWithFormat);
    }
}

void MainWindow::inspectorClosed()
{
    ui->actioninspector->setChecked(false);
}

void MainWindow::afterUpdateFrameIfPaused()
{
    if(stopAfterUpdate)
    {
        stopAfterUpdate = false;
        mPauseVidAfterUpdate();
    }
}

void MainWindow::setSliderValue(int valueS)
{
    if(!sliderPressed) ui->horizontalSlider->setValue(valueS);
}

bool MainWindow::checkifSave()
{
    if (!ifSave)
        return false;
    mSaveFrame();
    return true;
}

bool MainWindow::checkIfPaused()
{
    return ifPaused;
}

bool MainWindow::checkIfNewSliderValue()
{
    return ifNewSlierValue;
}

bool MainWindow::checkIfInitAborded()
{
    return initAborted;
}

void MainWindow::videoPlayerInit(QString videoPath, int numOfFram, double tc)
{
    mVideoInPutPath = videoPath;
    cutFileNameWithFormat();
    mFileName = mFileNameWithFormat.left(mFileNameWithFormat.indexOf("."));
    mNumOfFrames = numOfFram; //does it just once
    decoder.setVidInitPos(tc);
    decoder.decodeFile(mVideoInPutPath);

    if (mNumOfFrames == 0) mNumOfFrames = decoder.setDefaultFramNum();
    else decoder.setNumOfFrames(mNumOfFrames);
    ui->horizontalSlider->setRange(0, mNumOfFrames);
    decoder.start();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(onScreenPlayPause.contains(event->pos()))
    {
        ifPaused = !ifPaused;
        if(!ifPaused) ui->playPauseButton->setIcon(QIcon(mPauseIcon));
        else  ui->playPauseButton->setIcon(QIcon(mPlayIcon));
        decoder.setPausedPlay();
    }

    animation->blockSignals(true);
    animation->stop();
    animation->blockSignals(false);
    opacityEffect->setOpacity(1.0);

    if(auto fps = decoder.getVideoFps(); fps == fps) //skips when picture as an input
    ui->vidProgWidget->setVisible(true);
    timer->start(g_timeOut);
}

void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
    animation->blockSignals(true);
    animation->stop();
    animation->blockSignals(false);
    opacityEffect->setOpacity(1.0);

    if(auto fps = decoder.getVideoFps(); fps == fps) //skips when picture as an input
    ui->vidProgWidget->setVisible(true);
    timer->start(g_timeOut);

    QMainWindow::mouseMoveEvent(e);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
}

void MainWindow::on_actionexit_triggered()
{
    close();
}

void MainWindow::mSaveFrame()
{
    if(!ui->progressBar->isVisible())
        ui->progressBar->show();
    mFrameSaveCounter++;
    int tempVal = ((double) mFrameSaveCounter / mNumOfExportFrames) * 100;
    ui->progressBar->setValue(tempVal);
    update();

    mPauseVidAfterUpdate();
    int pos = mFileOutPut.lastIndexOf(".");
    QString temp = mFileOutPut;
    vidWidget->saveFrames(temp, mFrameSaveCounter, pos);
    mPlayVidToUpdate();

    if(mFrameSaveCounter == mNumOfExportFrames)
    {
        ifSave = false;
        ui->progressBar->hide();
        mPauseVidAfterUpdate();
        QMessageBox::information(this, "Status inspector", "All images saved successfully!");
        if(!stopAfterSaving)
            mPlayVidToUpdate();
        mFrameSaveCounter = 0;
    }
}

void MainWindow::on_actionSave_triggered()
{
    SaveFramesDialog saveNumFramesDialog;
    saveNumFramesDialog.setWindowTitle("Export settings");

    if (saveNumFramesDialog.exec() == QDialog::Rejected)
        return;

    mNumOfExportFrames = saveNumFramesDialog.numFramesToSave();

    mFileOutPut = saveNumFramesDialog.exportPath(mFileName);
    if(!mFileOutPut.isEmpty()) ifSave = true;
    if(ifPaused) stopAfterSaving = true;
    if(mNumOfExportFrames != 0 && ifSave)
        mSaveFrame();
}

void MainWindow::on_playPauseButton_clicked()
{
    if(ifPaused == false)
    {
        ui->playPauseButton->setIcon(QIcon(mPlayIcon));
        ifPaused = true;
    }
    else
    {
        ui->playPauseButton->setIcon(QIcon(mPauseIcon));
        ifPaused = false;
    }
    decoder.setPausedPlay();
}

void MainWindow::on_actionAbout_this_app_triggered()
{
     QMessageBox::about(this, "About this app",
                        "Current version: 1.2.0\nCreated by: Kamil Janko\n\n"
                        "This program demuxes and decodes the given video and allows to add watermark, onion skinning, change brightness,"
                        " manipulate RGB channels and save particular frames. Since the version 1.1 it is also possible to open pictures.");
}

void MainWindow::on_actionwater_mark_triggered()
{
    vidWidget->waterMark();
}

void MainWindow::on_actioninspector_triggered()
{
    ui->widgetInspector->show();

    //OLD
//    inspector.setWindowTitle("Inspector");
//    inspector.show();
}

//Gets the video file name
void MainWindow::cutFileNameWithFormat()
{
    int pos = mVideoInPutPath.length() - mVideoInPutPath.lastIndexOf('/') - 1;
    mFileNameWithFormat = mVideoInPutPath.right(pos);
}

void MainWindow::mPlayVidToUpdate()
{
    if(ifPaused)
    {
        ifPaused = false;
//        ui->playPauseButton->setIcon(QIcon(mPauseIcon));
        decoder.setPausedPlay();
    }
}

void MainWindow::mPauseVidAfterUpdate()
{
    if(!ifPaused)
    {
        ifPaused = true;
//        ui->playPauseButton->setIcon(QIcon(mPlayIcon));
        decoder.setPausedPlay();
    }
}

void MainWindow::hideInterface()
{
    animation->start();
}

void MainWindow::setVideoTimeCode(double videoTime)
{
    int minutes = videoTime / 60;
    int seconds = (int)videoTime % 60;
    seconds < 10 ? mSeconds = "0" + QString::number(seconds) : mSeconds = QString::number(seconds);
    minutes < 10 ? mMinutes = "0" + QString::number(minutes) : mMinutes = QString::number(minutes);
    mVideoCurrPos = mMinutes + ":" + mSeconds;
    ui->timelabel->setText(mVideoCurrPos);

    double videoRem = decoder.getVideoDuration() - videoTime;
    int rminutes = videoRem / 60;
    int rseconds = (int)videoRem % 60;
    rseconds < 10 ? mRemSeconds = "0" + QString::number(rseconds) : mRemSeconds = QString::number(rseconds);
    rminutes < 10 ? mRemMinutes = "0" + QString::number(rminutes) : mRemMinutes = QString::number(rminutes);
    mRemVideoTime = mRemMinutes + ":" + mRemSeconds;
    ui->labelRemainingTime->setText(mRemVideoTime);
    update();
}

void MainWindow::on_actiononion_skinning_triggered()
{
    vidWidget->onionSkinning();
}

void MainWindow::on_horizontalSlider_sliderPressed()
{
    if(ifPaused)
    {
        mPlayVidToUpdate();
        stopAfterUpdate = true;
    }
    sliderPressed = true;
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    if(sliderPressed)
    {
        decoder.setNewSliderValue(value);
        sliderPressed = false;
    }
}

void MainWindow::on_horizontalSlider_sliderReleased()
{
    sliderPressed = false;
    decoder.setNewSliderValue(ui->horizontalSlider->sliderPosition());
}

void MainWindow::on_horizontalSlider_sliderMoved(int position)
{
     decoder.setNewSliderValue(position);
}

void MainWindow::on_actionBlack_triggered()
{
    mBgColor = "black";
    update();
}

void MainWindow::on_actionWhite_triggered()
{
    mBgColor = "white";
    update();
}

void MainWindow::on_actioninspector_triggered(bool checked)
{
    if(!checked) ui->widgetInspector->close();
}
