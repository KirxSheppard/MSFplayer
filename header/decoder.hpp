#pragma once

#include <string>
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include "includeffmpeg.h"
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

    int setDefaultFramNum();
    void scrollVideo();
    void setVidInitPos(double tcode);
    void setNumOfFrames(int num);
    void setPausedPlay();
    void setNewSliderValue(int sliderValue);
    void run();
    void stop();
    void playerSleepThread();
    QImage imgToRGB();
    bool loopPlayCond(bool force = false);
    int getHeight();
    int getWidth();
    int getFrameIterator();
    double getVideoFps();


    //Slots
    void setBrightness(int value);
    void setBrightState(bool brightState);

    void setRedChannel(int value);
    void setRedChannelState(bool redState);

    void setGreenChannel(int value);
    void setGreenChannelState(bool greenState);

    void setBlueChannel(int value);
    void setBlueChannelState(bool blueState);



signals:
    void mRgb(const QImage &qimg);
    void positon(const int currPos);
    void videoTimeCode(const double videoTime);

private:
    void adjustColor(int initPos, int step, int value);
    void rgb2Hsv(int type, int valueToAdd);

private:
    static QString videoInPut;
    QString frameOutPut;
    QString msfLogo;
    double desiredPos;
    double userDesideredPos = 0.0;
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

    bool checkIfBrightness();
    bool hasFrameAfterSeek = false;
    bool canRead = true;
    bool fluhed = false;
    bool mStop = false;
    bool mifUserSetNewValue = false;
    bool mIfPaused = false;
    bool ifBrightOpt = false;
    bool ifRedOpt = false;
    bool ifGreenOpt = false;
    bool ifBlueOpt = false;

    int mNewSliderValue = -1;
    int mVidBright = 0; //how much to brighten an image
    int mVidRed = 0;
    int mVidGreen = 0;
    int mVidBlue = 0;

    double mVideoDuration = 0.0;
    uint8_t *data = nullptr;

//    QVector<QImage> qImg;
    QImage qImg;

    int mFrameIterator = 0;
    int mHeight = 0, mWidth = 0;

    QElapsedTimer et;

    QThread mThread;
    QMutex mMutex;
    QWaitCondition waitCond;

    double mPrevTime = 0.0;
};
