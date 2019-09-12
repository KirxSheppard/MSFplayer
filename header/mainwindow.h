#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QThread>
#include <QMutex>
#include "slider.hpp"
#include "decoder.hpp"
#include "videowidget.h"
#include "inspector.hpp"
#include "saveframesdialog.hpp"
#include "initialdialog.h"
#include <QtConcurrent/QtConcurrentRun>
#include <QtMath>
#include <QElapsedTimer>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

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
    bool checkIfNewSliderValue();
    bool checkIfInitAborded();

    void videoPlayerInit(QString videoPath, int numOfFram, double tc);
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void closeEvent(QCloseEvent *event) override;
    void rgb2Hsv(int type, int valueToAdd);
    void cutFileNameWithFormat();
    void mPlayVidToUpdate();
    void mPauseVidAfterUpdate();

    //Slots
    void setVideoTimeCode(double videoTime);
    void setSliderValue(int valueS);
    void inspectorClosed();
    void setVidSize(QSize val);
    void afterUpdateFrameIfPaused();
public slots:
    void hideInterface();
    void on_horizontalSlider_valueChanged(int value);
    void on_actionexit_triggered();    
private slots:
    void mSaveFrame();
    void on_actionSave_triggered();
    void on_playPauseButton_clicked();
    void on_actionAbout_this_app_triggered();

    void on_actionwater_mark_triggered();

    void on_actiononion_skinning_triggered();

    void on_horizontalSlider_sliderPressed();

    void on_horizontalSlider_sliderReleased();

    void on_horizontalSlider_sliderMoved(int position);

    void on_actionBlack_triggered();

    void on_actionWhite_triggered();

    void on_actioninspector_triggered();

    void on_actioninspector_triggered(bool checked);

private:
    Ui::MainWindow *ui;
    QPropertyAnimation  *animation;
    QGraphicsOpacityEffect *opacityEffect;
    InitialDialog initDialog;
    VideoWidget *vidWidget;

    Slider slider;
    QList<QImage> m_imgs;
    QImage m_imgGOps;
    QString mVideoInPutPath, mMsfLogoPath, mWaterMarkPath, mFileNameWithFormat, mFileOutPut, mFileName;
    bool ifExit, ifSave, ifPaused, ifNewSlierValue;
    int mNumOfFrames = -1;
    int mNumOfExportFrames, mFrameSaveCounter;
    Decoder decoder;
    int logoHeight = 200;
    int logoWidth = 200;
    int mNewSliderValue;
    int mWmPosX = 0, mWmPosY = 0;
    int mWmScale = 100;
    double mWmOpacityVal = 0.3;
    QString mMinutes, mSeconds, mRemMinutes, mRemSeconds;
    QString mVideoCurrPos = "00:00";
    QString mRemVideoTime = "00:00";
    QString mBgColor = "black";
    QString mPauseIcon = ":/resources/pause.png";
    QString mPlayIcon = ":/resources/play.png";
    QSize s;

    bool sliderPressed = false;
    bool initWindowWasUsed = false;
    bool initAborted = false;
    bool stopAfterUpdate = false;
    bool stopAfterSaving = false;

    QRect onScreenPlayPause;
    QTimer *timer;
};

#endif // MAINWINDOW_H
