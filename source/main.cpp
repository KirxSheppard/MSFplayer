#include <windows.h>
#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include "includeffmpeg.h"
#include "decoder.hpp"
#include <QThread>
#include <QImage>
#include <iostream>
#include <QtEndian>

const QString videoInPut = "E:/5_001_01.mov";
const QString frameOutPut = "C:/Users/Sheppard/Desktop/test/";
const double desiredPos = 2.34;
const int numOfFrames = 10;

AVFormatContext *fmtCtx = nullptr;
AVCodecContext *codexCtx = nullptr;
AVCodec *codec = nullptr;
AVStream *stream = nullptr;
AVPacket *pkt = nullptr;
AVFrame *frame = nullptr;
SwsContext *swsCtx = nullptr;

qint64 startTime = 0;
qint64 numFrames = 0;

bool hasFrameAfterSeek = false;




using namespace std;

enum class GetFrame
{
    Error,
    Again,
    Ok,
};

bool getPacket()
{
    // Packet is invalid when it returns false
    for (;;)
    {
        av_packet_unref(pkt);
        const int ret = av_read_frame(fmtCtx, pkt);
        if (ret < 0)
        {
            break;
        }
        if (pkt->stream_index == stream->index)
        {
            return true;
        }
    }
    return false;
}

GetFrame receiveFrame()
{
    int ret = 0;
    if(!hasFrameAfterSeek)
    {
        ret = avcodec_receive_frame(codexCtx, frame);
        Q_ASSERT(!frame->data[0] || frame->data[0]);
        if(ret == AVERROR(EAGAIN))
        {
            return GetFrame::Again;
        }
    }
    else hasFrameAfterSeek = false;
    return (ret==0) ? GetFrame::Ok : GetFrame::Error;
}

bool readFrame()
{
    if(receiveFrame() == GetFrame::Ok) return true;

    while((pkt->data != nullptr) || (getPacket()==true))
    {
        const int ret = avcodec_send_packet(codexCtx, pkt);
        av_packet_unref(pkt); // Now the packet data is nullptr

        if (ret == AVERROR(EAGAIN))
        {
            continue;
        }

        switch (receiveFrame())
        {
            case GetFrame::Again:
                continue;
            case GetFrame::Ok:
                return true;
            default:
                break;
        }

        return false;
    }

    if ((avcodec_send_packet(codexCtx, nullptr)) == 0 && (receiveFrame() == GetFrame::Ok))
    {
        return true;
    }

    return false;
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler([](QtMsgType t , const QMessageLogContext &c, const QString &s) {
        cerr << c.file << ":" << c.line << " " << qUtf8Printable(qFormatLogMessage(t, c, s)) << endl;
    });

    fmtCtx = avformat_alloc_context();
   // fmtCtx->flags |= AVFMT_FLAG_GENPTS; // potrzebne mi to?

    if(avformat_open_input(&fmtCtx, videoInPut.toUtf8().constData(), nullptr, nullptr) != 0) return 0;

    if(avformat_find_stream_info(fmtCtx, nullptr) < 0) return 0;

    AVCodec *decoder = nullptr;
    const int streamIdx = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);

    if((decoder == nullptr) || (streamIdx < 0)) return 0;

    stream = fmtCtx->streams[streamIdx];
    if(!stream) return 0;

    codexCtx = avcodec_alloc_context3(decoder);

    if(avcodec_parameters_to_context(codexCtx, stream->codecpar) < 0 ) return 0;

    //default usage of threads but most optimal:
    codexCtx->thread_count = QThread::idealThreadCount();
    codexCtx->thread_type = FF_THREAD_FRAME;


   if(avcodec_open2(codexCtx, codec, nullptr) < 0) return 0;

   pkt = av_packet_alloc();
   frame = av_frame_alloc();



   int64_t ts = stream->time_base.den * desiredPos / stream->time_base.num;
   cerr << ts << endl;
   if (av_seek_frame(fmtCtx, stream->index, ts, AVSEEK_FLAG_BACKWARD) >= 0)
   {
       avcodec_flush_buffers(codexCtx);
   }

   bool canRead = true;
   bool fluhed = false;
   int f = 0;
   AVFrame *prev = nullptr;
   for (;;)
   {
       int err;

       if (canRead)
       {
           err = av_read_frame(fmtCtx, pkt);
           if (err != 0)
           {
               canRead = false;
           }


//           qDebug() <<"pts: "<<pkt->pts;
//           qDebug() <<"dts: "<<pkt->dts;
//           qDebug() << bool(pkt->flags & AV_PKT_FLAG_KEY)
//                    << av_get_picture_type_char(frame->pict_type);

//           qDebug("packet flags: %d picture type: %c\n", pkt->flags,
//                      av_get_picture_type_char(frame->pict_type));

           if (pkt->stream_index != stream->index)
           {
               av_packet_unref(pkt);
               continue;
           }
       }

       if (!fluhed)
       {
           err = avcodec_send_packet(codexCtx, pkt);

           if (pkt->data == nullptr && pkt->size == 0)
               fluhed = true;

           av_packet_unref(pkt);

           if (err == AVERROR(EAGAIN))
               continue;
           else if (err != 0)
               break;
       }

       err = avcodec_receive_frame(codexCtx, frame);
       if (err == AVERROR(EAGAIN))
           continue;
       else if (err != 0)
           break;

       const double currPos = av_q2d(stream->time_base) * frame->best_effort_timestamp;

       if (currPos < desiredPos)
           continue;

       qDebug() << currPos
                << av_get_picture_type_char(frame->pict_type)
                << frame->key_frame;

       QImage grayScale;

       auto pixFmtDescr = av_pix_fmt_desc_get((AVPixelFormat)frame->format);
       if (pixFmtDescr && pixFmtDescr->comp[0].depth > 8)
       {
           grayScale = QImage(frame->width, frame->height, QImage::Format_Grayscale8); //do zmiany gdy chce kolorki

           uint16_t *src = (uint16_t *)frame->data[0];
           uint16_t *src2 = (uint16_t *)(prev ? prev : frame)->data[0];

           int dstLineSize = grayScale.bytesPerLine();
           uint8_t *dst = grayScale.bits();

           for (int y = 0; y < frame->height; ++y)
           {
               for (int x = 0; x < frame->width; ++x)
               {
                   double p1 = src[y * frame->linesize[0] / 2 + x] / 1023.0;
                   double p2 = src2[y * frame->linesize[0] / 2 + x] / 1023.0;

                   double p = (p1 + p2) / 2.0;

                   dst[y * dstLineSize + x] = clamp(p, 0.0, 1.0) * 255;

//                   p >>= 2;
//                   dst[y * dstLineSize + x] = p;
               }
           }
       }
       else
       {
           grayScale = QImage(frame->data[0], frame->width, frame->height, frame->linesize[0], QImage::Format_Grayscale8);
       }
       grayScale.save(QString(frameOutPut + "TE%1.tiff").arg(f, 6, 10, QLatin1Char('0')));

       if (!prev)
           prev = av_frame_alloc();
       else
           av_frame_unref(prev);
       av_frame_ref(prev, frame);

       ++f;
       if(f==numOfFrames) break; //saves only the given number of frames
    }

   av_frame_free(&prev);

  // return 0;

    int width = codexCtx->width;
    int height = codexCtx->height;

    QApplication a(argc, argv);
    MainWindow w;
    w.setFixedSize(width/2, height/2);
    w.show();
    w.setWindowTitle("TE000000.tiff");

    qDebug("%X", avformat_version());

    avcodec_flush_buffers(codexCtx);
    return a.exec();
}
