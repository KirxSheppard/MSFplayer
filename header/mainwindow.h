#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QThread>
#include <QMutex>
#include "slider.hpp"
#include "decoder.hpp"
#include "brightnessdialog.hpp"
#include "saveframesdialog.hpp"
#include "initialdialog.h"
#include <QtConcurrent/QtConcurrentRun>
#include <QtMath>
#include <QElapsedTimer>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const QString msfLogo, QWidget *parent = nullptr);
    ~MainWindow();

    int setFrame();
    bool checkifSave();
    bool checkIfPaused();
    bool checkWaterMark();
    bool checkIfNewSliderValue();
    bool checkIfInitAborded();

    void videoPlayer(QString videoPath, int numOfFram, double tc);
    void videoFrameLoop();
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void closeEvent(QCloseEvent *event) override;
    void rgb2Hsv(int type, int valueToAdd);
    void cutFileNameWithFormat();

    //Slots
    void setVideoTimeCode(double videoTime);
    void setSliderValue(int valueS);
    void setImage(const QImage &img);
public slots:
    void hideInterface();
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
    InitialDialog initDialog;

    Slider slider;
    QList<QImage> m_imgs;
    QImage m_imgGOps;
    QString mVideoInPutPath, mMsfLogoPath, mFileNameWithFormat, mFileOutPut, mFileName;
    bool ifExit, ifSave, ifPaused, ifWaterMark, ifBrightOpt, ifNewSlierValue, ifOnionSkinning, ifRedOpt, ifGreenOpt, ifBlueOpt, ifOnScreenPressed;
    int height, width;
    int mNumOfFrames = -1;
    int mNumOfExportFrames, mFrameSaveCounter;
    Decoder decoder;
    int logoHeight = 200;
    int logoWidth = 200;
    int mNewSliderValue;
    QString mMinutes, mSeconds, mRemMinutes, mRemSeconds;
    QString mVideoCurrPos = "00:00";
    QString mRemVideoTime = "00:00";

    bool sliderPressed = false;
    bool initWindowWasUsed = false;
    bool initAborted = false;

    QRect onScreenPlayPause;
    QTimer *timer;
};

#endif // MAINWINDOW_H
