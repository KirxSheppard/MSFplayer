#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <iostream>
#include <QFileDialog>

MainWindow::MainWindow(const QString fileName, const QString videoInPut, const QString msfLogo, QWidget *parent) :
    QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    ui->statusbar->setStyleSheet("color: white");
    ui->statusbar->showMessage(fileName);
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
    ui->playPauseButton->setStyleSheet(QString::fromUtf8("background-color: argb(255,0,0,0);"));
    ui->playPauseButton->setIcon(QIcon(":/resources/pause.png"));
    ui->timelabel->setStyleSheet("color: white");
    ui->timelabel->setText(mVideoCurrPos);

    ifSave = false;
    ifPaused = false;
    ifWaterMark = false;
    ifBrightOpt = false;
    ifRedOpt = false;
    ifGreenOpt = false;
    ifBlueOpt = false;
    ifNewSlierValue = false;
    ifOnionSkinning =false;

    numOfFrames = decoder.getNumOfFrames();
    frameSaveCounter = 0;
    mVideoInPutPath = videoInPut;
    mMsfLogoPath = msfLogo;
    mFileNameWithFormat = fileName;

    mFileName = fileName.left(fileName.indexOf("."));


//    qRegisterMetaType<QVector<QImage>>();

    connect(&decoder, &Decoder::mRgb,
            this, &MainWindow::setImage);
    connect(&decoder, &Decoder::positon,
            this, &MainWindow::setSliderValue);
    connect(&brightnessDialog, &BrightnessDialog::ifBrightBool,
            this, &MainWindow::setBrightState);
    connect(&brightnessDialog, &BrightnessDialog::brightValue,
            this, &MainWindow::setBrightness);
    connect(&brightnessDialog, &BrightnessDialog::ifRedChannel,
            this, &MainWindow::setRedChannelState);
    connect(&brightnessDialog, &BrightnessDialog::redValue,
            this, &MainWindow::setRedChannel);
    connect(&brightnessDialog, &BrightnessDialog::ifGreenChannel,
            this, &MainWindow::setGreenChannelState);
    connect(&brightnessDialog, &BrightnessDialog::greenValue,
            this, &MainWindow::setGreenChannel);
    connect(&brightnessDialog, &BrightnessDialog::ifBlueChannel,
            this, &MainWindow::setBlueChannelState);
    connect(&brightnessDialog, &BrightnessDialog::blueValue,
            this, &MainWindow::setBlueChannel);
    connect(&decoder, &Decoder::videoTimeCode,
            this, &MainWindow::setVideoTimeCode);

//    QVector<int> a;
//    a.resize(1093294432);

//    QVector<int> b = a;

//    int *d = b.data();
//    d[];
}

MainWindow::~MainWindow()
{
    decoder.stop();
    decoder.wait();
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    ui->statusbar->showMessage(mFileNameWithFormat);

    if (m_imgs.size() != 2)
        return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.fillRect(this->rect(),QColor("black"));
    const QSize imgS =  m_imgs[1].size();
    QSize s = imgS.scaled(size(), Qt::KeepAspectRatio);

    painter.drawImage(QRect(QPoint(), s), m_imgs[1]);
    if (ifOnionSkinning)
    {
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.setOpacity(0.3);
        painter.drawImage(QRect(QPoint(), s), m_imgs[0]);
    }
    if(checkWaterMark() == true)
    {
        QImage waterMarkImg(mMsfLogoPath); //should be declared just once
        painter.setCompositionMode(QPainter::CompositionMode_Xor);
        painter.setOpacity(0.4);
        painter.drawImage(s.width() / 2 - waterMarkImg.width() / 2,  s.height() / 2 - waterMarkImg.height() / 2, waterMarkImg);
    }
}

void MainWindow::setImage(const QImage &img)
{
    if (m_imgs.size() >= 2)
        m_imgs.removeFirst();
    m_imgs.push_back(img);

    if(checkIfBrightness())
    {
//        adjustColor(0, 1, mVidBright);
        rgb2Hsv(0, mVidBright); //temp values and place
    }
    if(ifRedOpt)
    {
        adjustColor(2, 4, mVidRed);
    }
    if(ifGreenOpt)
    {
        adjustColor(1, 4, mVidGreen);
    }
    if(ifBlueOpt)
    {
        adjustColor(0, 4, mVidBlue);
    }


    if (checkifSave())
    {
        int pos = mFileOutPut.lastIndexOf(".");
        QString temp = mFileOutPut;
        m_imgs[1].save(QString(temp.insert(pos,"%1")).arg(frameSaveCounter, 6, 10, QLatin1Char('0')));
    }
    update();
}

void MainWindow::setBrightState(bool brightState)
{
    ifBrightOpt = brightState;
}

void MainWindow::setRedChannel(int value)
{
    mVidRed = value;
}

void MainWindow::setRedChannelState(bool redState)
{
    ifRedOpt = redState;
}

void MainWindow::setGreenChannel(int value)
{
    mVidGreen = value;
}

void MainWindow::setGreenChannelState(bool greenState)
{
    ifGreenOpt = greenState;
}

void MainWindow::setBlueChannel(int value)
{
    mVidBlue = value;
}

void MainWindow::setBlueChannelState(bool blueState)
{
    ifBlueOpt = blueState;
}

void MainWindow::setSliderValue(int valueS)
{
    if(!sliderPressed)
    {
        valueS = 100 / (double) numOfFrames * valueS;
        ui->horizontalSlider->setValue(valueS);
    }
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
            frameSaveCounter = 0;
        }
    }
    return ifSave;
}

bool MainWindow::checkIfPaused()
{
    return ifPaused;
}

bool MainWindow::checkWaterMark()
{
    return ifWaterMark;
}

bool MainWindow::checkIfBrightness()
{
    return ifBrightOpt;
}

bool MainWindow::checkIfNewSliderValue()
{
    return ifNewSlierValue;
}

void MainWindow::videoPlayer()
{
    decoder.decodeFile(mVideoInPutPath);
}


void MainWindow::resizeEvent(QResizeEvent *event)
{
    //    HANDLE id = (HANDLE)winId(); //for the future work to keep aspect ratio of the main window
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    brightnessDialog.close();
}

void MainWindow::on_actionexit_triggered()
{
    close();
}

void MainWindow::on_actionSave_triggered()
{   
    mFileOutPut = QFileDialog::getSaveFileName(this, "Save file", mFileName, tr("Images (*.png *.xpm *.jpg);; TIFF (*.tiff);; All files (*.*)"));

    if(mFileOutPut != nullptr) ifSave = true;
}

void MainWindow::on_playPauseButton_clicked()
{
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
    decoder.setPausedPlay();
}

void MainWindow::on_actionAbout_this_app_triggered()
{
     QMessageBox::about(this, "About this app", "This is the first MSF's GUI app fully written in C++. This program demuxes and decodes the given video and allows to add watermark, onion skinning, change brightness and save particular frames.");
}

void MainWindow::on_actionwater_mark_triggered()
{
    ifWaterMark = !ifWaterMark;
}

void MainWindow::on_actionbrightness_triggered()
{
//    brightOpt = new BrightnessDialog(this);
    brightnessDialog.setWindowTitle("Color inspector");
    brightnessDialog.setFixedSize(300,280);
    brightnessDialog.show();
}

//Adjust brightness and RGB intensity
void MainWindow::adjustColor(int initPos, int step, int value)
{
    uint8_t *data = m_imgs[1].bits();
    int w4 = m_imgs[1].width() * 4;
    int h = m_imgs[1].height();
    int ls = m_imgs[1].bytesPerLine();

    const int nTasks = QThread::idealThreadCount();

    auto task = [&](int i) {
        const int begin = (i + 0) * h / nTasks;
        const int end   = (i + 1) * h / nTasks;
        for (int y = begin; y < end; y += 1)
        {
            for (int x = initPos; x < w4; x += step)
            {
                data[y * ls + x] = clamp((int)data[y * ls + x] + value, 0, 255);
            }
        }
    };

    QVector<QFuture<void>> thrs(nTasks - 1);

    for (int i = 0; i < thrs.size(); ++i)
        thrs[i] = QtConcurrent::run(task, i);
    task(thrs.size());

    for (auto &&t : thrs)
        t.waitForFinished();
}

void MainWindow::rgb2Hsv(int type, int value)
{
    uint8_t *data = m_imgs[1].bits();

    double max, min;
    int w4 = m_imgs[1].width() * 4;
    int h = m_imgs[1].height();
    int ls = m_imgs[1].bytesPerLine();

    const int nTasks = QThread::idealThreadCount();

    auto task = [&](int i) {
        const int begin = (i + 0) * h / nTasks;
        const int end   = (i + 1) * h / nTasks;
        for (int y = begin; y < end; y += 1)
        {
            for (int r = 2, g = 1, b = 0; r < w4; r += 4, g += 4, b+= 4)
            {
                red = (double) (clamp((int)data[y * ls + r], 0, 255)) / 255;
                green = (double) (clamp((int)data[y * ls + g], 0, 255)) / 255;
                blue = (double) (clamp((int)data[y * ls + b], 0, 255)) / 255;

                min = red < green ? red : green;
                min = min < blue  ? min : blue;

                max = red > green ? red : green;
                max = max > blue  ? max : blue;


                //Hue
                if(red >= green && red >= blue)
                {
                    if(red == green && green == blue) hue = 0.0;

                    else hue = (60 * M_PI / 360) * ((green - blue) / (max - min));
                }
                else if (green >= red && green >= blue)
                {
                    if(red == green && green == blue) hue = 0.0;
                    else hue = (60 * M_PI / 360) * (2 + (blue - green) / (max - min));
                }
                else if (blue >= red && blue >= green)
                {
                    if(red == green && green == blue) hue = 0.0;
                    else hue = (60 * M_PI / 360) * (4 + (red - green) / (max - min));
                }


                //Saturation
                if(max == 0) saturation = 0.0;
                else
                {
                    saturation = (max - min) / max;
                }

                //Value
                value = max;

            }
        }
    };

    QVector<QFuture<void>> thrs(nTasks - 1);

    for (int i = 0; i < thrs.size(); ++i)
        thrs[i] = QtConcurrent::run(task, i);
    task(thrs.size());

    for (auto &&t : thrs)
        t.waitForFinished();
}

void MainWindow::setVideoTimeCode(double videoTime)
{
    int minutes = videoTime / 60;
    int seconds = (int)videoTime % 60;
    seconds < 10 ? mSeconds = "0" + QString::number(seconds) : mSeconds = QString::number(seconds);
    minutes < 10 ? mMinutes = "0" + QString::number(minutes) : mMinutes = QString::number(minutes);
    mVideoCurrPos = mMinutes + ":" + mSeconds;
    ui->timelabel->setText(mVideoCurrPos);
}

void MainWindow::setBrightness(int value)
{
    mVidBright = value;
}

void MainWindow::on_actiononion_skinning_triggered()
{
    ifOnionSkinning = !ifOnionSkinning;
}

void MainWindow::on_horizontalSlider_sliderPressed()
{
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

