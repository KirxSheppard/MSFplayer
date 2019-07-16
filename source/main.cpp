#include <windows.h>
#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include "includeffmpeg.h"
#include "decoder.hpp"
#include <QThread>
#include <QImage>
#include <iostream>

const QString videoInPut = "E:/Episode 2.m4v";
const QString frameOutPut = "C:/Users/Sheppard/Desktop/";


AVFormatContext *fmtCtx = NULL;
AVCodecContext *codexCtx = NULL;
AVCodec *codec = NULL;
AVStream *stream = NULL;
AVPacket *pkt = NULL;
AVFrame *frame = NULL;
SwsContext *swsCtx = NULL;

qint64 startTime = 0;
qint64 numFrames = 0;

bool eof = false;

bool hasFrameAfterSeek = false;

using namespace std;
/*
bool BMPSave(const char *pFileName, AVFrame * frame, int w, int h)

{

    bool bResult = false;



    if (frame)

    {

        FILE* file = fopen(pFileName, "wb");

        if (file)

        {

            // RGB image

            int imageSizeInBytes = 3 * w * h;

            int headersSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

            int fileSize = headersSize + imageSizeInBytes;



            uint8_t * pData = new uint8_t[headersSize];



            if (pData != NULL)

            {

                BITMAPFILEHEADER& bfHeader = *((BITMAPFILEHEADER *)(pData));




                bfHeader.bfType = 0x4D42; // WORD('M' << 8) | 'B';

                bfHeader.bfSize = fileSize;

                bfHeader.bfOffBits = headersSize;

                bfHeader.bfReserved1 = bfHeader.bfReserved2 = 0;



                BITMAPINFOHEADER& bmiHeader = *((BITMAPINFOHEADER *)(pData + headersSize - sizeof(BITMAPINFOHEADER)));



                bmiHeader.biBitCount = 3 * 8;

                bmiHeader.biWidth    = w;

                bmiHeader.biHeight   = h;

                bmiHeader.biPlanes   = 1;

                bmiHeader.biSize     = sizeof(bmiHeader);

                bmiHeader.biCompression = BI_RGB;

                bmiHeader.biClrImportant = bmiHeader.biClrUsed =

                    bmiHeader.biSizeImage = bmiHeader.biXPelsPerMeter =

                    bmiHeader.biYPelsPerMeter = 0;
                fwrite(pData, headersSize, 1, file);

                uint8_t *pBits = frame->data[0] + frame->linesize[0] * h - frame->linesize[0];
                int nSpan = frame->linesize[0];

                int numberOfBytesToWrite = 3 * w;

                for (size_t i = 0; i < h; ++i, pBits -= nSpan)
                {

                    fwrite(pBits, numberOfBytesToWrite, 1, file);
                }
                bResult = true;

                delete [] pData;
            }
            fclose(file);
        }

    }



    return bResult;

}
*/


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
            if (ret == AVERROR_EOF)
            {
                eof = true;
            }
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

    while((pkt->data != NULL) || (getPacket()==true))
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

    fmtCtx = avformat_alloc_context();
   // fmtCtx->flags |= AVFMT_FLAG_GENPTS; // potrzebne mi to?

    if(avformat_open_input(&fmtCtx, videoInPut.toUtf8().constData(), nullptr, nullptr) != 0) return 0;

    if(avformat_find_stream_info(fmtCtx, NULL) < 0) return 0;

    AVCodec *decoder = NULL;
    const int streamIdx = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);

    if((decoder == NULL) || (streamIdx < 0)) return 0;

    stream = fmtCtx->streams[streamIdx];
    if(!stream) return 0;

    codexCtx = avcodec_alloc_context3(decoder);

    if(avcodec_parameters_to_context(codexCtx, stream->codecpar) < 0 ) return 0;

    //default usage of threads but most optimal:
    codexCtx->thread_count = QThread::idealThreadCount();
    codexCtx->thread_type = FF_THREAD_FRAME;


   if(avcodec_open2(codexCtx, codec, NULL) < 0) return 0;

   pkt = av_packet_alloc();
   frame = av_frame_alloc();

   int64_t ts = stream->time_base.den * 60.34 / stream->time_base.num;
   cerr << ts << endl;
   if (av_seek_frame(fmtCtx, stream->index, ts, AVSEEK_FLAG_BACKWARD) >= 0)
       avcodec_flush_buffers(codexCtx);

   bool canRead = true;
   bool fluhed = false;
   int f = 0;
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

       QImage grayScale(frame->data[0], frame->width, frame->height, frame->linesize[0], QImage::Format_Grayscale8);
       grayScale.save(QString(frameOutPut + "test/temp%1.tiff").arg(f, 6, 10, QLatin1Char('0')));

       ++f;
    }

    int width = codexCtx->width;
    int height = codexCtx->height;


    QApplication a(argc, argv);
    MainWindow w;
    w.setFixedSize(width, height);
    w.show();
    w.setWindowTitle("FFMPEG frame decoder");




    /*
    //Decoder decoder;
    Decoder *decoder = new Decoder();

    if(decoder->openFile(videoInPut))
    {
        width = decoder->getWidth();
        height = decoder->getHeight();

        for (int i = 0; i < 3; i++)

            {

              AVFrame * frame = decoder->GetNextFrame();

              if (frame)

              {

                QString asd = frameOutPut + QString::number(i) + ".bmp";

                if (!BMPSave(asd.toUtf8().constData(), frame, frame->width, frame->height))
                {

                  qDebug() << "Cannot save file" << asd;

                }

                av_free(frame->data[0]);

                av_free(frame);

              }

            }
    }





    */


    qDebug("%X", avformat_version());

    avcodec_flush_buffers(codexCtx);
    return a.exec();
}
