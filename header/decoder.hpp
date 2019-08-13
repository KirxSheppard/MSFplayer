#pragma once

#include <string>
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include "includeffmpeg.h"
#include "brightnessdialog.hpp"
#include <QImage>
#include <iostream>
#include <QElapsedTimer>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QGuiApplication>

using namespace std;

class Decoder : public QThread //previously it was QObject
{
    Q_OBJECT

public:
    Decoder();
    ~Decoder();

    enum class GetFrame
    {
        Error,
        Again,
        Ok,
    };

    virtual bool decodeFile(const QString& videoInPut);
    bool getPacket();
    bool readFrame();
    double getVideoDuration();
    int getNumOfFrames();
    GetFrame receiveFrame();

    void scrollVideo();
    void setVidInitPos(double tcode);
    void setNumOfFrames(int num);
    void setPausedPlay();
    void setNewSliderValue(int sliderValue);
    void run();
    void stop();
    void playerSleepThread();
    QImage imgToRGB();
    void loopPlayCond();
    int getHeight();
    int getWidth();
    int getFrameIterator();

signals:
    void mRgb(const QImage &qimg);
    void positon(const int currPos);
    void videoTimeCode(const double videoTime);

private:
    static QString videoInPut;
    QString frameOutPut;
    QString msfLogo;
    double desiredPos;
    double userDesideredPos;
    int numOfFrames;

    AVFormatContext *fmtCtx = nullptr;
    AVCodecContext *codexCtx = nullptr;
    AVCodec *codec = nullptr;
    AVCodec *decoder = nullptr;
    AVStream *stream = nullptr;
    AVPacket *pkt = nullptr;
    AVFrame *frame = nullptr;
    struct SwsContext *swsCtx = nullptr;

    qint64 startTime = 0;
    qint64 numFrames = 0;

    bool hasFrameAfterSeek = false;
    bool canRead = true;
    bool fluhed = false;
    bool mStop = false;
    bool mIfPaused = false;
    int mNewSliderValue = -1;
    bool mifUserSetNewValue = false;

    double mVideoDuration = 0.0;
    uint8_t *data = nullptr;

//    QVector<QImage> qImg;
    QImage qImg;

    int mFrameIterator = 0;
    int mHeight = 0, mWidth = 0;

    BrightnessDialog brightDialog;

    QElapsedTimer et;

    QThread mThread;
    QMutex mMutex;
    QWaitCondition waitCond;

    double mPrevTime = 0.0;
};
