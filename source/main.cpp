#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include "includeffmpeg.h"
#include <QImage>
#include <iostream>
#include <QElapsedTimer>
#include <QThread>

const QString videoInPut = "E:/5_001_01.mov";
const QString frameOutPut = "C:/Users/Sheppard/Desktop/test/";
const QString msfLogo = "E:/ikona msf small.png";
const double desiredPos = 0;
const int numOfFrames = 10;

const int logoHeight = 200;
const int logoWidth = 200;

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
        cerr << c.line << " " << qUtf8Printable(qFormatLogMessage(t, c, s)) << endl;
    });

    QApplication a(argc, argv);

    MainWindow w;
    w.show();

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

//   int64_t ts = stream->time_base.den * desiredPos / stream->time_base.num;
//   cerr << ts << endl;
//   if (av_seek_frame(fmtCtx, stream->index, ts, AVSEEK_FLAG_BACKWARD) >= 0)
//   {
//       avcodec_flush_buffers(codexCtx);
//   }

   bool canRead = true;
   bool fluhed = false;
   int f = 0;
   AVFrame *prev = nullptr;

   QElapsedTimer et;

   QImage waterMark(msfLogo);
   //qDebug() << waterMark.format();

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
           uint16_t *src2 = (uint16_t *)(prev ? prev : frame)->data[0]; //second layer

           int logoLineSize = waterMark.bytesPerLine();
           uint8_t *srcLogo = waterMark.bits();

           int dstLineSize = grayScale.bytesPerLine();
           uint8_t *dst = grayScale.bits();

           int yLogo = 0;
           for (int y = 0; y < frame->height; ++y)
           {
               if(y > frame->height - logoHeight - 20) ++yLogo;
               int xLogo = 0;

               for (int x = 0; x < frame->width; ++x)
               {                  
                   if(x > frame->width - logoWidth) ++xLogo;

                   double p1 = src[y * frame->linesize[0] / 2 + x] / 1023.0;
                   double p2 = src2[y * frame->linesize[0] / 2 + x] / 1023.0;


                   double p = (p1 + p2) / 2.0;
//                   p += 0.3; //brightness

                   if(xLogo < logoWidth  &&  yLogo < logoHeight )
                   {
                       double p3a = srcLogo[yLogo * logoLineSize + xLogo * 4 + 0] / 255.0;
                       double p3 = srcLogo[yLogo * logoLineSize + xLogo * 4 + 3] / 255.0;
                      // p = p * p3a + p3 * p3a * (1.0 - p3a);
                       p+=p3;
                   }


                   dst[y * dstLineSize + x] = clamp(p, 0.0, 1.0) * 255;

//                   p >>= 2;
//                   dst[y * dstLineSize + x] = p;
               }
           }
       }
       else
       {
           grayScale = QImage( frame->width, frame->height, QImage::Format_Grayscale8);

           uint8_t *src = (uint8_t *)frame->data[0];
           uint8_t *src2 = (uint8_t *)(prev ? prev : frame)->data[0]; //second layer

           int logoLineSize = waterMark.bytesPerLine();
           uint8_t *srcLogo = waterMark.bits();

           int dstLineSize = grayScale.bytesPerLine();
           uint8_t *dst = grayScale.bits();


           int yLogo = 0;

           for (int y = 0; y < frame->height; ++y)
           {

               if(y > frame->height - logoHeight-20) ++yLogo;
               int xLogo = 0;

               for (int x = 0; x < frame->width; ++x)
               {

                   if(x > frame->width - logoWidth) ++xLogo;

                   double p1 = src[y * frame->linesize[0] + x] / 255.0;
                   double p2 = src2[y * frame->linesize[0] + x] / 255.0;

                   double p = (p1 + p2) / 2.0;
                   //p += 0.3; //brightness

                   if(xLogo < logoWidth  &&  yLogo < logoHeight)
                   {

                        //double p3 = srcLogo[y * logoLineSize + x * 4 + 0] / 255.0;
                        double p3a = srcLogo[yLogo * logoLineSize + xLogo * 4 + 3] / 255.0;

                       // p = p * p3a + p3*(1.0-p3a); //alpha channel
                       // p = fmod(p * p3a + p3 * p3a * (1.0 - p3a),(p3a + (1.0 - p3a)));
                        p += p3a;
                   }
                   dst[y * dstLineSize + x] = clamp(p, 0.0, 1.0) * 255;
               }
           }
       }

       if (et.isValid() && prev)
       {
           double t = et.nsecsElapsed() / 1e9;

           double frameTime = av_q2d(stream->time_base) * (frame->best_effort_timestamp - prev->best_effort_timestamp);

           int sleepTime = (frameTime - t) * 1000;
           if (sleepTime > 0)
               QThread::msleep(sleepTime);
       }
       et.start();

       int width = codexCtx->width;
       int height = codexCtx->height;

       w.setFixedSize(width/2, height/2);
       w.setImage(grayScale);
       a.processEvents();

       //grayScale.save(QString(frameOutPut + "TE%1.tiff").arg(f, 6, 10, QLatin1Char('0'))); //time consuming

       if (!prev)
           prev = av_frame_alloc();
       else
       av_frame_unref(prev);
       av_frame_ref(prev, frame);

       ++f;
//       if(f==numOfFrames) break; //saves only the given number of frames
    }

    av_frame_free(&prev);

    return a.exec();
}
