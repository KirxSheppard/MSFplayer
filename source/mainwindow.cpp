#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <iostream>
#include <QFileDialog>
#include <cmath>
#include <QVector4D>
#include <QVector3D>
#include <QTimer>
#include <cctype>

constexpr int g_timeOut = 4000;

MainWindow::MainWindow(const QString msfLogo, QWidget *parent) :
    QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->start(g_timeOut);

    ui->setupUi(this);
    ui->statusbar->setStyleSheet("color: white");
    ui->statusbar->showMessage(mFileNameWithFormat);
    ui->statusbar->addPermanentWidget(ui->progressBar);
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
    ifWaterMark = false;
    ifBrightOpt = false;
    ifRedOpt = false;
    ifGreenOpt = false;
    ifBlueOpt = false;
    ifNewSlierValue = false;
    ifOnionSkinning =false;
//    ifOnScreenPressed = false;

    mFrameSaveCounter = 0;
    mMsfLogoPath = msfLogo;
    mWaterMarkPath = mMsfLogoPath;

    connect(timer, &QTimer::timeout, this, &MainWindow::hideInterface);

    connect(&decoder, &Decoder::videoTimeCode,
            this, &MainWindow::setVideoTimeCode);
    connect(&decoder, &Decoder::mRgb,
            this, &MainWindow::setImage);
    connect(&decoder, &Decoder::positon,
            this, &MainWindow::setSliderValue);
    connect(&inspector, &Inspector::ifBrightBool,
            &decoder, &Decoder::setBrightState);
    connect(&inspector, &Inspector::brightValue,
            &decoder, &Decoder::setBrightness);
    connect(&inspector, &Inspector::ifRedChannel,
            &decoder, &Decoder::setRedChannelState);
    connect(&inspector, &Inspector::redValue,
            &decoder, &Decoder::setRedChannel);
    connect(&inspector, &Inspector::ifGreenChannel,
            &decoder, &Decoder::setGreenChannelState);
    connect(&inspector, &Inspector::greenValue,
            &decoder, &Decoder::setGreenChannel);
    connect(&inspector, &Inspector::ifBlueChannel,
            &decoder, &Decoder::setBlueChannelState);
    connect(&inspector, &Inspector::blueValue,
            &decoder, &Decoder::setBlueChannel);
    connect(&inspector, &Inspector::wmOpacityValue,
            this, &MainWindow::setWmOpacityValue);
    connect(&inspector, &Inspector::rejected,
            this, &MainWindow::inspectorClosed);
    connect(&inspector, &Inspector::wmPosX,
            this, &MainWindow::setWmPosX);
    connect(&inspector, &Inspector::wmPosY,
            this, &MainWindow::setWmPosY);
    connect(&inspector, &Inspector::wmSetPressed,
            this, &MainWindow::setWmPath);
    connect(&inspector, &Inspector::wmResetPath,
            this, &MainWindow::resetWmPath);
    connect(&inspector, &Inspector::wmScaleValue,
            this, &MainWindow::setWmScaleValue);
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

    //background
    p.fillRect(this->rect(), QColor(mBgColor));

    p.drawImage(QRect(QPoint((width() - s.width()) / 2, 0), s), m_imgGOps);

    //On screen rect for play/pause button
    onScreenPlayPause = QRect(s.width() / 4,
                              s.height() / 4,
                              s.width() / 2,
                              s.height() / 2);

    //here it works fine without any weird glitches except Windows
#ifndef Q_OS_WINDOWS
    this->resize(s.width(),s.height());
#endif

    if(auto fps = decoder.getVideoFps(); fps == fps)
    {
        ui->statusbar->showMessage(mFileNameWithFormat + " (" + QString::number(decoder.getVideoFps()) + "fps)");
    }
    else {
        ui->vidProgWidget->hide();
        ui->statusbar->showMessage(mFileNameWithFormat);
    }
}

void MainWindow::setImage(const QImage &img)
{
    //only two items (current and prev)
    if (m_imgs.size() >= 2)
        m_imgs.removeFirst();
    else if (m_imgs.empty()) //allows to display photos
        m_imgs.push_back(img);
    m_imgs.push_back(img);

    if (m_imgs.size() != 2) return;

    if (m_imgGOps.size() != m_imgs[1].size() || m_imgGOps.format() != m_imgs[1].format())
        m_imgGOps = QImage(m_imgs[1].size(), m_imgs[1].format());

    updateVidEffects();
}

void MainWindow::setWmOpacityValue(double value)
{
    mWmOpacityVal = value;
    updateVidEffects();
}

void MainWindow::setWmScaleValue(int value)
{
    mWmScale = value;
    updateVidEffects();
}

void MainWindow::inspectorClosed()
{
    ui->actioninspector->setChecked(false);
}

void MainWindow::setWmPosX(int value)
{
    mWmPosX = value;
    updateVidEffects();
}

void MainWindow::setWmPosY(int value)
{
    mWmPosY = value;
    updateVidEffects();
}

void MainWindow::resetWmPath(bool value)
{
    if(value)
    {
        mWaterMarkPath = mMsfLogoPath;
        updateVidEffects();
    }
}

void MainWindow::setWmPath(QString path)
{
    mWaterMarkPath = path;
}

void MainWindow::setSliderValue(int valueS)
{
    if(!sliderPressed) ui->horizontalSlider->setValue(valueS);
}

bool MainWindow::checkifSave()
{
    if (!ifSave)
        return false;

    ui->progressBar->show();
    mFrameSaveCounter++;
    int temp = ((double) mFrameSaveCounter / mNumOfExportFrames) * 100;
    ui->progressBar->setValue(temp);
    update();

    if(mFrameSaveCounter == mNumOfExportFrames)
    {
        ifSave = false;
        ui->progressBar->hide();
        QMessageBox::information(this, "Status inspector", "All images saved successfully!");
        mFrameSaveCounter = 0;
    }
    return true;
}

bool MainWindow::checkIfPaused()
{
    return ifPaused;
}

bool MainWindow::checkWaterMark()
{
    return ifWaterMark;
}

bool MainWindow::checkIfNewSliderValue()
{
    return ifNewSlierValue;
}

bool MainWindow::checkIfInitAborded()
{
    return initAborted;
}

void MainWindow::videoPlayer(QString videoPath, int numOfFram, double tc)
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
    inspector.close();
}

void MainWindow::on_actionexit_triggered()
{
    close();
}

void MainWindow::updateVidEffects()
{
    QPainter painter(&m_imgGOps);

    const QSize imgS = m_imgGOps.size();

    painter.drawImage(QRect(QPoint(), imgS), m_imgs[1]);

    if (ifOnionSkinning)
    {
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.setOpacity(0.3);
        painter.drawImage(QRect(QPoint(), imgS), m_imgs[0]);
    }
    if(checkWaterMark())
    {
        QImage waterMarkImg(mWaterMarkPath);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.setOpacity(mWmOpacityVal);
        if(waterMarkImg.height() > imgS.height())
        {
            inspector.resetWm2BigPic();
            mWaterMarkPath = mMsfLogoPath;
        }
        else
        {
            //here i can control water mark position and scale
            painter.drawImage(imgS.width()  / 2 - waterMarkImg.width()  * ((double)mWmScale / 100) / 2 + mWmPosX,
                              imgS.height() / 2 - waterMarkImg.height() * ((double)mWmScale / 100) / 2 + mWmPosY,
                              waterMarkImg.scaled(waterMarkImg.width()  * ((double)mWmScale / 100),
                                                  waterMarkImg.height() * ((double)mWmScale / 100),
                                                  Qt::KeepAspectRatio));
        }
    }
    painter.end();
    if(stopAfterUpdate)
    {
        stopAfterUpdate = false;
        mPauseVidAfterUpdate();
    }
    update();

    if (checkifSave())
    {
        mPauseVidAfterUpdate();
        int pos = mFileOutPut.lastIndexOf(".");
        QString temp = mFileOutPut;
        m_imgGOps.save(QString(temp.insert(pos,"_%1")).arg(mFrameSaveCounter, 6, 10, QLatin1Char('0')));
        mPlayVidToUpdate();
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
    updateVidEffects();
    update();
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
     QMessageBox::about(this, "About this app", "Current version: 1.1.1\nCreated by: Kamil Janko\n\nThis program demuxes and decodes the given video and allows to add watermark, onion skinning, change brightness, manipulate RGB channels and save particular frames. Since version 1.1 it is also possible to open pictures.");
}

void MainWindow::on_actionwater_mark_triggered()
{
    ifWaterMark = !ifWaterMark;
    updateVidEffects();
    update();
}

void MainWindow::on_actioninspector_triggered()
{
    inspector.setWindowTitle("Inspector");
    inspector.show();
}

// Not used anywhere because it uses too much energy to process the image in real time on the processor
void MainWindow::rgb2Hsv(int type, int valueToAdd)
{
    QElapsedTimer et;
    et.start();

    uint8_t *data = m_imgs[1].bits();

    const int w4 = m_imgs[1].width() * 4;
    const int h = m_imgs[1].height();
    const int ls = m_imgs[1].bytesPerLine();

    const int nTasks = QThread::idealThreadCount();

    auto task = [&](int i) {
        const int begin = (i + 0) * h / nTasks;
        const int end   = (i + 1) * h / nTasks;
        for (int y = begin; y < end; y += 1)
        {
            for (int r = 2, g = 1, b = 0; r < w4; r += 4, g += 4, b+= 4)
            {
                float blue = data[y * ls + b] / 255.0f;
                float green = data[y * ls + g] / 255.0f;
                float red = data[y * ls + r] / 255.0f;
#if 1
                auto mix = [](const QVector4D &x, const QVector4D &y, float fa) {
                    QVector4D a(fa, fa, fa, fa);
                    return x * (QVector4D(1.0f, 1.0f, 1.0f, 1.0f) - a) + y * a;
                };

                auto step = [](float edge, float x) {
                    return (x < edge) ? 0.0f : 1.0f;
                };
// rgba
// xyzw
                QVector3D hsv;

                {
                    QVector3D c(red, green, blue);

                    QVector4D K = QVector4D(0.0f, -1.0f / 3.0f, 2.0f / 3.0f, -1.0f);
                    QVector4D p = mix(QVector4D(c.z(), c.y(), K.w(), K.z()), QVector4D(c.y(), c.z(), K.x(), K.y()), step(c.z(), c.y()));
                    QVector4D q = mix(QVector4D(p.x(), p.y(), p.w(), c.x()), QVector4D(c.x(), p.y(), p.z(), p.x()), step(p.x(), c.x()));

                    float d = q.x() - std::min(q.w(), q.y());
                    float e = 1.0e-10f;

                    hsv = QVector3D(std::abs(q.z() + (q.w() - q.y()) / (6.0f * d + e)), d / (q.x() + e), q.x());
                }

                hsv[1] = clamp(hsv[1] + valueToAdd / 255.0f, 0.0f, 1.0f);

                auto fract = [](const QVector3D &v) {
                    return QVector3D(v.x() - std::floor(v.x()), v.y() - std::floor(v.y()), v.z() - std::floor(v.z()));
                };
                auto clamp = [](const QVector3D &v, float min, float max) {
                    return QVector3D(std::clamp(v.x(), min, max), std::clamp(v.y(), min, max), std::clamp(v.z(), min, max));
                };
                auto abs = [](const QVector3D &v) {
                    return QVector3D(std::abs(v.x()), std::abs(v.y()), std::abs(v.z()));
                };

                QVector3D rgb;
                {
                    QVector4D K = QVector4D(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
                    QVector3D Kxxx = QVector3D(K.x(), K.x(), K.x());
                    QVector3D p = abs(fract(QVector3D(hsv.x(), hsv.x(), hsv.x()) + QVector3D(K.x(), K.y(), K.z())) * QVector3D(6.0f, 6.0f, 6.0f) - QVector3D(K.w(), K.w(), K.w()));
                    rgb = QVector3D(hsv.z() * mix(Kxxx, clamp(p - Kxxx, 0.0, 1.0), hsv.y()));
                }

                data[y * ls + b] = std::clamp(rgb.z() * 255.0f, 0.0f, 255.0f);
                data[y * ls + g] = std::clamp(rgb.y() * 255.0f, 0.0f, 255.0f);
                data[y * ls + r] = std::clamp(rgb.x() * 255.0f, 0.0f, 255.0f);

#else
                float min = std::min({red, green, blue});
                float max = std::max({red, green, blue});

                float hue, saturation, value;

                //Hue
                if(max == red)
                {
                    if(red == green && green == blue) hue = 0.0f;

                    else hue = (60.0f * (float)M_PI / 360.0f) * ((green - blue) / (max - min));
                }
                if (green == max)
                {
                    if(red == green && green == blue) hue = 0.0f;
                    else hue = (60.0f * (float)M_PI / 360.0f) * (2.0f + (blue - green) / (max - min));
                }
                if (blue == max)
                {
                    if(red == green && green == blue) hue = 0.0f;
                    else hue = (60.0f * (float)M_PI / 360.0f) * (4.0f + (red - green) / (max - min));
                }
                if(red == green && green == blue) hue = 0.0f;

                if (hue < 0.0f)
                    hue += 360.0f * (float)M_PI / 360.0f;
//                hue += (float) valueToAdd / 255.0f; //temp

                //Saturation
                if(qFuzzyIsNull(max))
                {
                    saturation = 0.0f;
                }
                else
                {
                    saturation = (max - min) / max;
                }

                saturation += (float) valueToAdd / 255.0f;

                //Value
                value = max;

//                value += valueToAdd;

               //Conversion back to RGB after changes
               int huePrim = hue * 360; //that is what a documentation says about hue'
               float c = value * saturation;
               float x = c * (1.0f - std::abs(fmod(huePrim / 60.0f, 2.0f) - 1.0f)); //how to handle this?????
               float m = value - saturation;

               struct tempRGB
               {
                   float r;
                   float g;
                   float b;
               };
               tempRGB hsv2;

               if(qFuzzyIsNull(hue))
               {
                   hsv2.r = 0.0f;
                   hsv2.g = 0.0f;
                   hsv2.b = 0.0f;
               }
               else if(huePrim >= 0 && huePrim <= 60)
               {
                   hsv2.r = c;
                   hsv2.g = x;
                   hsv2.b = 0.0;
               }
               else if(huePrim > 60 && huePrim <= 120)
               {
                   hsv2.r = x;
                   hsv2.g = c;
                   hsv2.b = 0.0;
               }
               else if(huePrim > 120 && huePrim <= 180)
               {
                   hsv2.r = 0.0;
                   hsv2.g = c;
                   hsv2.b = x;
               }
               else if(huePrim > 180 && huePrim <= 240)
               {
                   hsv2.r = 0.0;
                   hsv2.g = x;
                   hsv2.b = c;
               }
               else if(huePrim > 240 && huePrim <= 300)
               {
                   hsv2.r = x;
                   hsv2.g = 0.0;
                   hsv2.b = c;
               }
               else
               {
                   hsv2.r = c;
                   hsv2.g = 0.0;
                   hsv2.b = x;
               }

               data[y * ls + b] = clamp(qRound((hsv2.b + m) * 255.0f), 0, 255);
               data[y * ls + g] = clamp(qRound((hsv2.g + m) * 255.0f), 0, 255);
               data[y * ls + r] = clamp(qRound((hsv2.r + m) * 255.0f), 0, 255);
#endif
            }
        }
    };

    QVector<QFuture<void>> thrs(nTasks - 1);

    for (int i = 0; i < thrs.size(); ++i)
        thrs[i] = QtConcurrent::run(task, i);
    task(thrs.size());

    for (auto &&t : thrs)
        t.waitForFinished();
    qDebug()<<et.nsecsElapsed() / 1e6 << "ms";
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
    ifOnionSkinning = !ifOnionSkinning;
    updateVidEffects();
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
    if(!checked) inspector.close();
}
