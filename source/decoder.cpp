#include "decoder.hpp"

#include <QtConcurrent/QtConcurrentRun>

Decoder::Decoder()
{
    startTime = 0;
    numFrames = 0;
    mFrameIterator = 0;

    hasFrameAfterSeek = false;
}

Decoder::~Decoder()
{
//    av_frame_free(&frame); //cleaning but here it's lags entire program on exit
}

bool Decoder::getPacket()
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

Decoder::GetFrame Decoder::receiveFrame()
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

int Decoder::setDefaultFramNum()
{

    numOfFrames = getVideoDuration() * getVideoFps() - getVideoFps() * desiredPos;
//       qDebug()<<"numOffra "<<numOfFrames<<" end";
    return numOfFrames;
}

void Decoder::scrollVideo()
{
    //new value set at the slider by the user
    if(mifUserSetNewValue)
    {
        int64_t ts = stream->time_base.den * userDesideredPos / stream->time_base.num;
        cerr << ts << endl;
        if (av_seek_frame(fmtCtx, stream->index, ts, AVSEEK_FLAG_BACKWARD) >= 0)
        {
            avcodec_flush_buffers(codexCtx);
        }
//        desiredPos = userDesideredPos;
    }
    mifUserSetNewValue = false;
}

void Decoder::setVidInitPos(double tcode)
{
    desiredPos = tcode;
}

void Decoder::setNumOfFrames(int num)
{
    numOfFrames = num;
}

void Decoder::setPausedPlay()
{
    mIfPaused = !mIfPaused;
//    waitCond.wakeAll();
}

void Decoder::run()
{
    for(;;)
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

        scrollVideo();

        err = avcodec_receive_frame(codexCtx, frame);
        if (err == AVERROR(EAGAIN))
        {
            if (!canRead && fluhed)
            {
                if (!loopPlayCond(true))
                    break;
            }
            continue;
        }
        else if (err != 0)
        {
            break;
        }

        const double currPos = av_q2d(stream->time_base) * frame->best_effort_timestamp;


        if (currPos < desiredPos)
           continue;


        if (!swsCtx)
        {
            swsCtx = sws_getContext(
                 frame->width,
                 frame->height,
                 (AVPixelFormat)frame->format,
                 frame->width,
                 frame->height,
                 AV_PIX_FMT_BGRA,
                 SWS_BILINEAR,
                 nullptr,
                 nullptr,
                 nullptr
             );
        }

        //log
        qDebug() << currPos
                 << av_get_picture_type_char(frame->pict_type)
                 << frame->key_frame;

        imgToRGB();

        emit videoTimeCode(currPos);
        emit mRgb(qImg);

        loopPlayCond();
        emit positon(getFrameIterator()); //currPos if we want to display progress bar in the total video length
        if(mIfPaused)
        {
            for(;;)
            {
//                mMutex.lock();
//                waitCond.wait(&mMutex);
//                mMutex.unlock();
                sleep(1);
                if(!mIfPaused || mStop) break;
            }
        }
//        playerSleepThread();
        if(mStop) break;
    }
}

void Decoder::stop()
{
    mStop = true;
//    waitCond.wakeAll();
}

void Decoder::playerSleepThread()
{
    double time = av_q2d(stream->time_base) * (frame->best_effort_timestamp);

    if (et.isValid() && mPrevTime > 0.0)
    {
        double t = et.nsecsElapsed() / 1e9;

        int sleepTime = (time - mPrevTime - t) * 1000;
//        qDebug()<<"st: "<< sleepTime;
        if (sleepTime > 0)
            QThread::msleep(sleepTime);
    }
    et.start();

    mPrevTime = time;
}

//Uses sws_scale for conversion for current frame
QImage Decoder::imgToRGB()
{
//        if (qImgs[i].format())
        qImg = QImage(frame->width, frame->height, QImage::Format_ARGB32);

        uint8_t *dstData[] {
            qImg.bits()
        };
        int dstLinesize[] {
            qImg.bytesPerLine()
        };

        sws_scale(
             swsCtx,
             frame->data,
             frame->linesize,
             0,
             frame->height,
             dstData,
             dstLinesize
         );
    return qImg;
}

bool Decoder::loopPlayCond(bool force)
{
    bool ok = false;
    ++mFrameIterator;
//    qDebug()<<"mFrame"<<mFrameIterator<<"numof"<<numOfFrames;
   if(force || mFrameIterator >= numOfFrames)// break; //runs only the given number of frames
   {
//       int64_t ts = stream->time_base.den * desiredPos / stream->time_base.num; //old
//       qDebug()<<ts;
       int64_t ts = av_q2d(stream->avg_frame_rate) * desiredPos;
       cerr << ts << endl;
       if (av_seek_frame(fmtCtx, stream->index, ts, AVSEEK_FLAG_BACKWARD) >= 0)
       {
           avcodec_flush_buffers(codexCtx);

           fluhed = false;
           canRead = false;

           ok = true;
       }
       mFrameIterator = 0;
   }

   return ok;
}

bool Decoder::readFrame()
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

double Decoder::getVideoDuration()
{
    //Returns total video duration in seconds
    mVideoDuration =  1.0 * fmtCtx->duration / (AV_TIME_BASE * 1.0);
//    qDebug()<<"Video duration: "<< mVideoDuration; //no need for this in the future
    return mVideoDuration;
}

int Decoder::getNumOfFrames()
{
    return numOfFrames;
}

bool Decoder::decodeFile(const QString &videoInPut)
{
    fmtCtx = avformat_alloc_context();

    if(avformat_open_input(&fmtCtx, videoInPut.toUtf8().constData(), nullptr, nullptr) != 0) return false;
    if(avformat_find_stream_info(fmtCtx, nullptr) < 0) return false;

    const int streamIdx = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);

    if((decoder == nullptr) || (streamIdx < 0)) return 0;

    stream = fmtCtx->streams[streamIdx];
    if(!stream) return 0;

    codexCtx = avcodec_alloc_context3(decoder);

    if(avcodec_parameters_to_context(codexCtx, stream->codecpar) < 0 ) return 0;

    //Default usage of threads but most optimal:
    codexCtx->thread_count = QThread::idealThreadCount();
    codexCtx->thread_type = FF_THREAD_FRAME;

   if(avcodec_open2(codexCtx, codec, nullptr) < 0) return 0;

   pkt = av_packet_alloc();
   frame = av_frame_alloc();

   int64_t ts = stream->time_base.den * (mifUserSetNewValue ? userDesideredPos : desiredPos) / stream->time_base.num; //choose default or new slider
   cerr << ts << endl;
   if (av_seek_frame(fmtCtx, stream->index, ts, AVSEEK_FLAG_BACKWARD) >= 0)
   {
       avcodec_flush_buffers(codexCtx);
   }

//   start();
   return true;
}

int Decoder::getWidth()
{
    if(codexCtx != nullptr)
        mWidth = codexCtx->width;
    return mWidth;
}

int Decoder::getFrameIterator()
{
    return mFrameIterator;
}

double Decoder::getVideoFps()
{
    return av_q2d(stream->avg_frame_rate);
}

void Decoder::setNewSliderValue(int sliderValue)
{
    mifUserSetNewValue = true;
    userDesideredPos = desiredPos + sliderValue / getVideoFps();
    mFrameIterator = sliderValue;
}

int Decoder::getHeight()
{
    if(codexCtx != nullptr)
        mHeight = codexCtx->height;
    return mHeight;
}
