#include "decoder.hpp"

#include <QtConcurrent/QtConcurrentRun>
#include <QVector4D>
#include <QVector3D>
#include <cmath>

Decoder::Decoder()
{
    startTime = 0;
    numFrames = 0;
    mFrameIterator = 0;

    hasFrameAfterSeek = false;
}

Decoder::~Decoder()
{
    av_frame_free(&frame);
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
//        cerr <<"ts: " <<ts << endl;
        if (av_seek_frame(fmtCtx, stream->index, ts, AVSEEK_FLAG_BACKWARD) >= 0)
        {
            avcodec_flush_buffers(codexCtx);
        }
        mPrevTime = 0.0;
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
                qDebug() << "Read frame error:" << err;
            }
            else if (pkt->stream_index != stream->index)
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
            {
                continue;
            }
            else if (err != 0)
            {
                qDebug() << "Send packet error:" << err;
                break;
            }
        }

        scrollVideo();

        err = avcodec_receive_frame(codexCtx, frame);
        if (err == AVERROR(EAGAIN))
        {
            continue;
        }
        else if (err != 0)
        {
//            char errBuff[AV_ERROR_MAX_STRING_SIZE];
//            av_make_error_string(errBuff, sizeof(errBuff), err);
//            qDebug() << "Receive frame error:" << err << errBuff << -541478725 << AVERROR(EOF);

            if (!loopPlayCond(true))
                break;
            continue;
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
//        qDebug() << currPos
//                 << av_get_picture_type_char(frame->pict_type)
//                 << frame->key_frame;

        imgToRGB();

        {
            if(checkIfBrightness())
            {
                adjustColor(0, 1, mVidBright);
            }
            if(ifRedOpt)
            {

                adjustColor(2, 4, mVidRed);
            }
            if(ifGreenOpt)
            {
                adjustColor(1, 4, mVidGreen);
            }
            if(ifBlueOpt)
            {
                adjustColor(0, 4, mVidBlue);
            }
        }

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
        playerSleepThread();
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
   if(force || mFrameIterator >= numOfFrames) //runs only the given number of frames
   {
       int64_t ts = av_q2d(stream->avg_frame_rate) * desiredPos;
       cerr <<"ts: "<<ts << endl;
       if (av_seek_frame(fmtCtx, stream->index, ts, AVSEEK_FLAG_BACKWARD) >= 0)
       {
           avcodec_flush_buffers(codexCtx);

           fluhed = false;
           canRead = true;

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

   if(desiredPos != 0.0)
   {
       int64_t ts = stream->time_base.den * (mifUserSetNewValue ? userDesideredPos : desiredPos) / stream->time_base.num; //choose default or new slider
       cerr <<"ts: " <<ts << endl;
       if (av_seek_frame(fmtCtx, stream->index, ts, AVSEEK_FLAG_BACKWARD) >= 0)
       {
           avcodec_flush_buffers(codexCtx);
       }
   }
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

//Adjust brightness and RGB intensity
void Decoder::adjustColor(int initPos, int step, int value)
{
    uint8_t *src = qImg.bits();
    uint8_t *dst = qImg.bits();

    int w4 = qImg.width() * 4;
    int h = qImg.height();
    int ls = qImg.bytesPerLine();

    const int nTasks = QThread::idealThreadCount();

    auto task = [&](int i) {
        const int begin = (i + 0) * h / nTasks;
        const int end   = (i + 1) * h / nTasks;
        for (int y = begin; y < end; y += 1)
        {
            for (int x = initPos; x < w4; x += step)
            {
                dst[y * ls + x] = clamp((int)src[y * ls + x] + value, 0, 255);
            }
        }
    };

    QVector<QFuture<void>> thrs(nTasks - 1);

    for (int i = 0; i < thrs.size(); ++i)
        thrs[i] = QtConcurrent::run(task, i);
    task(thrs.size());

    for (auto &&t : thrs)
        t.waitForFinished();
}

// Not used anywhere because it uses too much energy to process the image in real time on the processor, will be added to the next releases.
void Decoder::rgb2Hsv(int type, int valueToAdd)
{
    QElapsedTimer et;
    et.start();

    uint8_t *data = qImg.bits();

    const int w4 = qImg.width() * 4;
    const int h = qImg.height();
    const int ls = qImg.bytesPerLine();

    const int nTasks = QThread::idealThreadCount();

    auto task = [&](int i) {
        const int begin = (i + 0) * h / nTasks;
        const int end   = (i + 1) * h / nTasks;
        for (int y = begin; y < end; y += 1)
        {
            for (int r = 2, g = 1, b = 0; r < w4; r += 4, g += 4, b+= 4)
            {
                float blue = data[y * ls + b] / 255.0f;
                float green = data[y * ls + g] / 255.0f;
                float red = data[y * ls + r] / 255.0f;
#if 1
                auto mix = [](const QVector4D &x, const QVector4D &y, float fa) {
                    QVector4D a(fa, fa, fa, fa);
                    return x * (QVector4D(1.0f, 1.0f, 1.0f, 1.0f) - a) + y * a;
                };

                auto step = [](float edge, float x) {
                    return (x < edge) ? 0.0f : 1.0f;
                };
// rgba
// xyzw
                QVector3D hsv;

                {
                    QVector3D c(red, green, blue);

                    QVector4D K = QVector4D(0.0f, -1.0f / 3.0f, 2.0f / 3.0f, -1.0f);
                    QVector4D p = mix(QVector4D(c.z(), c.y(), K.w(), K.z()), QVector4D(c.y(), c.z(), K.x(), K.y()), step(c.z(), c.y()));
                    QVector4D q = mix(QVector4D(p.x(), p.y(), p.w(), c.x()), QVector4D(c.x(), p.y(), p.z(), p.x()), step(p.x(), c.x()));

                    float d = q.x() - std::min(q.w(), q.y());
                    float e = 1.0e-10f;

                    hsv = QVector3D(std::abs(q.z() + (q.w() - q.y()) / (6.0f * d + e)), d / (q.x() + e), q.x());
                }

                hsv[1] = clamp(hsv[1] + valueToAdd / 255.0f, 0.0f, 1.0f);

                auto fract = [](const QVector3D &v) {
                    return QVector3D(v.x() - std::floor(v.x()), v.y() - std::floor(v.y()), v.z() - std::floor(v.z()));
                };
                auto clamp = [](const QVector3D &v, float min, float max) {
                    return QVector3D(std::clamp(v.x(), min, max), std::clamp(v.y(), min, max), std::clamp(v.z(), min, max));
                };
                auto abs = [](const QVector3D &v) {
                    return QVector3D(std::abs(v.x()), std::abs(v.y()), std::abs(v.z()));
                };

                QVector3D rgb;
                {
                    QVector4D K = QVector4D(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
                    QVector3D Kxxx = QVector3D(K.x(), K.x(), K.x());
                    QVector3D p = abs(fract(QVector3D(hsv.x(), hsv.x(), hsv.x()) + QVector3D(K.x(), K.y(), K.z())) * QVector3D(6.0f, 6.0f, 6.0f) - QVector3D(K.w(), K.w(), K.w()));
                    rgb = QVector3D(hsv.z() * mix(Kxxx, clamp(p - Kxxx, 0.0, 1.0), hsv.y()));
                }

                data[y * ls + b] = std::clamp(rgb.z() * 255.0f, 0.0f, 255.0f);
                data[y * ls + g] = std::clamp(rgb.y() * 255.0f, 0.0f, 255.0f);
                data[y * ls + r] = std::clamp(rgb.x() * 255.0f, 0.0f, 255.0f);

#else
                float min = std::min({red, green, blue});
                float max = std::max({red, green, blue});

                float hue, saturation, value;

                //Hue
                if(max == red)
                {
                    if(red == green && green == blue) hue = 0.0f;

                    else hue = (60.0f * (float)M_PI / 360.0f) * ((green - blue) / (max - min));
                }
                if (green == max)
                {
                    if(red == green && green == blue) hue = 0.0f;
                    else hue = (60.0f * (float)M_PI / 360.0f) * (2.0f + (blue - green) / (max - min));
                }
                if (blue == max)
                {
                    if(red == green && green == blue) hue = 0.0f;
                    else hue = (60.0f * (float)M_PI / 360.0f) * (4.0f + (red - green) / (max - min));
                }
                if(red == green && green == blue) hue = 0.0f;

                if (hue < 0.0f)
                    hue += 360.0f * (float)M_PI / 360.0f;
//                hue += (float) valueToAdd / 255.0f; //temp

                //Saturation
                if(qFuzzyIsNull(max))
                {
                    saturation = 0.0f;
                }
                else
                {
                    saturation = (max - min) / max;
                }

                saturation += (float) valueToAdd / 255.0f;

                //Value
                value = max;

//                value += valueToAdd;

               //Conversion back to RGB after changes
               int huePrim = hue * 360; //that is what a documentation says about hue'
               float c = value * saturation;
               float x = c * (1.0f - std::abs(fmod(huePrim / 60.0f, 2.0f) - 1.0f)); //how to handle this?????
               float m = value - saturation;

               struct tempRGB
               {
                   float r;
                   float g;
                   float b;
               };
               tempRGB hsv2;

               if(qFuzzyIsNull(hue))
               {
                   hsv2.r = 0.0f;
                   hsv2.g = 0.0f;
                   hsv2.b = 0.0f;
               }
               else if(huePrim >= 0 && huePrim <= 60)
               {
                   hsv2.r = c;
                   hsv2.g = x;
                   hsv2.b = 0.0;
               }
               else if(huePrim > 60 && huePrim <= 120)
               {
                   hsv2.r = x;
                   hsv2.g = c;
                   hsv2.b = 0.0;
               }
               else if(huePrim > 120 && huePrim <= 180)
               {
                   hsv2.r = 0.0;
                   hsv2.g = c;
                   hsv2.b = x;
               }
               else if(huePrim > 180 && huePrim <= 240)
               {
                   hsv2.r = 0.0;
                   hsv2.g = x;
                   hsv2.b = c;
               }
               else if(huePrim > 240 && huePrim <= 300)
               {
                   hsv2.r = x;
                   hsv2.g = 0.0;
                   hsv2.b = c;
               }
               else
               {
                   hsv2.r = c;
                   hsv2.g = 0.0;
                   hsv2.b = x;
               }

               data[y * ls + b] = clamp(qRound((hsv2.b + m) * 255.0f), 0, 255);
               data[y * ls + g] = clamp(qRound((hsv2.g + m) * 255.0f), 0, 255);
               data[y * ls + r] = clamp(qRound((hsv2.r + m) * 255.0f), 0, 255);
#endif
            }
        }
    };

    QVector<QFuture<void>> thrs(nTasks - 1);

    for (int i = 0; i < thrs.size(); ++i)
        thrs[i] = QtConcurrent::run(task, i);
    task(thrs.size());

    for (auto &&t : thrs)
        t.waitForFinished();
    qDebug()<<et.nsecsElapsed() / 1e6 << "ms";
}

void Decoder::setBrightness(int value)
{
    mVidBright = value;
}

void Decoder::setBrightState(bool brightState)
{
    ifBrightOpt = brightState;
}

void Decoder::setRedChannel(int value)
{
    mVidRed = value;
}

void Decoder::setRedChannelState(bool redState)
{
    ifRedOpt = redState;
}

void Decoder::setGreenChannel(int value)
{
    mVidGreen = value;
}

void Decoder::setGreenChannelState(bool greenState)
{
    ifGreenOpt = greenState;
}

void Decoder::setBlueChannel(int value)
{
    mVidBlue = value;
}

void Decoder::setBlueChannelState(bool blueState)
{
    ifBlueOpt = blueState;
}

bool Decoder::checkIfBrightness()
{
    return ifBrightOpt;
}

