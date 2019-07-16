#ifndef DECODER_HPP
#define DECODER_HPP

#include <string>
#include "includeffmpeg.h"
#include <QApplication>
#include <QDebug>

using namespace std;

class Decoder
{
public:
    Decoder();


    virtual bool openFile(const QString& videoInPut);
    bool closeFile();

    int getWidth();
    int getHeight();
    virtual AVFrame *GetNextFrame();

private:
    int mWidth;
    int mHeight;
    int videoStreamIndex;
    double fps;
    double ffmpegTimeBase;
    bool isOpen;

    AVCodec* mVideoCodec;
    AVCodecContext* mVideoCodecCtx;
    AVFormatContext* mFormatCtx;
    struct SwsContext *mImgConvertCtx;

    AVFrame * GetRGBAFrame(AVFrame *mFrameYuv);
    bool decodeVideo(const AVPacket *avpkt, AVFrame * mOutFrame);


    bool openVideo();
    void closeVideo();



};

#endif // DECODER_HPP
