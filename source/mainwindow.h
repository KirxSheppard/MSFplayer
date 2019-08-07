#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QThread>
#include <QMutex>
#include "slider.hpp"
#include "decoder.hpp"
#include "brightnessdialog.hpp"
#include <QtConcurrent/QtConcurrentRun>
#include <QtMath>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const QString fileName, const QString videoInPut, const QString msfLogo, QWidget *parent = nullptr);
    ~MainWindow();

    int setFrame();
    bool checkifSave();
    bool checkIfPaused();
    bool checkWaterMark();
    bool checkIfBrightness();
    bool checkIfNewSliderValue();

    void videoPlayer();
    void videoFrameLoop();
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void closeEvent(QCloseEvent *event);
    void adjustColor(int initPos, int step, int value);
    void rgb2Hsv(int type, int value);

    //Slots
    void setVideoTimeCode(double videoTime);
    void setSliderValue(int valueS);
    void setImage(const QImage &img);
    void setBrightness(int value);
    void setBrightState(bool brightState);

    void setRedChannel(int value);
    void setRedChannelState(bool redState);

    void setGreenChannel(int value);
    void setGreenChannelState(bool greenState);

    void setBlueChannel(int value);
    void setBlueChannelState(bool blueState);
public slots:
    void on_horizontalSlider_valueChanged(int value);
    void on_actionexit_triggered();    
private slots:
    void on_actionSave_triggered();
    void on_playPauseButton_clicked();
    void on_actionAbout_this_app_triggered();
    void on_actionwater_mark_triggered();

    void on_actionbrightness_triggered();

    void on_actiononion_skinning_triggered();

    void on_horizontalSlider_sliderPressed();

    void on_horizontalSlider_sliderReleased();

    void on_horizontalSlider_sliderMoved(int position);

private:
    Ui::MainWindow *ui;
    BrightnessDialog brightnessDialog;
    Slider slider;
    QList<QImage> m_imgs;
    QString mVideoInPutPath, mMsfLogoPath, mFileNameWithFormat, mFileOutPut, mFileName;
    bool ifExit, ifSave, ifPaused, ifWaterMark, ifBrightOpt, ifNewSlierValue, ifOnionSkinning, ifRedOpt, ifGreenOpt, ifBlueOpt;
    int height, width;
    int numOfFrames, frameSaveCounter;
    Decoder decoder;
    int mVidBright = 0; //how much to brighten an image
    int mVidRed = 0;
    int mVidGreen = 0;
    int mVidBlue = 0;
    int logoHeight = 200;
    int logoWidth = 200;
    int mNewSliderValue;
    QString mMinutes, mSeconds;
    QString mVideoCurrPos = "00:00";

    bool sliderPressed = false;
//    bool canSetValue = true;

    double red, green, blue;
    double hue, saturation, value;
};

#endif // MAINWINDOW_H
