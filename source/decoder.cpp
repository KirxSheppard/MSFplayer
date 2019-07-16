#include "decoder.hpp"

Decoder::Decoder()
{
    mFormatCtx = NULL;
    mVideoCodec = NULL;
    mVideoCodecCtx = NULL;
    mImgConvertCtx = NULL;
    videoStreamIndex = -1;
    isOpen = false;
    ffmpegTimeBase = 0.0;
    fps = 0.0;
}

bool Decoder::openFile(const QString &videoInPut)
{

    closeFile();

    //av_register_all();


    if(avformat_open_input(&mFormatCtx, videoInPut.toUtf8().constData(), NULL, NULL) != 0)
    {
        closeFile();
        return false;
    }
    if(avformat_find_stream_info(mFormatCtx, NULL) < 0)
    {
        closeFile();
        return false;
    }

    bool hasVideo = openVideo();

    if(!hasVideo)
    {
        closeFile();
        return false;
    }

    isOpen = true;


    // Get file information.
        if (videoStreamIndex != -1)
        {
            fps = av_q2d(mFormatCtx->streams[videoStreamIndex]->r_frame_rate);

            // Need for convert time to ffmpeg time.
            ffmpegTimeBase = av_q2d(mFormatCtx->streams[videoStreamIndex]->time_base);
        }



    return true;
}

bool Decoder::closeFile()
{
    isOpen = false;



        // Close video
        closeVideo();


        if (mFormatCtx)

        {

            avformat_close_input(&mFormatCtx);
            mFormatCtx = NULL;

        }
        return true;
}

int Decoder::getWidth()
{
    return mWidth;
}

int Decoder::getHeight()
{
    return mHeight;
}

AVFrame *Decoder::GetNextFrame()
{
    AVFrame * res = NULL;



        if (videoStreamIndex != -1)

        {

            AVFrame *pVideoYuv = av_frame_alloc();

            AVPacket packet;



            if (isOpen)

            {

                // Read packet.

                while (av_read_frame(mFormatCtx, &packet) >= 0)

                {

                    int64_t pts = 0;

                    pts = (packet.dts != AV_NOPTS_VALUE) ? packet.dts : 0;



                    if(packet.stream_index == videoStreamIndex)

                    {

                        // Convert ffmpeg frame timestamp to real frame number.

                        int64_t numberFrame = (double)((int64_t)pts -

                            mFormatCtx->streams[videoStreamIndex]->start_time) *

                            ffmpegTimeBase * fps;



                        // Decode frame

                        bool isDecodeComplite = decodeVideo(&packet, pVideoYuv);

                        if (isDecodeComplite)

                        {

                            res = GetRGBAFrame(pVideoYuv);

                        }

                        break;

                    }

                    av_free_packet(&packet);

                    packet = AVPacket();

                }



                av_free(pVideoYuv);

            }

        }



        return res;
}

AVFrame *Decoder::GetRGBAFrame(AVFrame *mFrameYuv)
{
    AVFrame * frame = NULL;

        int width  = mVideoCodecCtx->width;

        int height = mVideoCodecCtx->height;

        int bufferImgSize = avpicture_get_size(AV_PIX_FMT_BGR24, width, height);

        frame = av_frame_alloc();

        uint8_t * buffer = (uint8_t*)av_mallocz(bufferImgSize);

        if (frame)

        {

            avpicture_fill((AVPicture*)frame, buffer, AV_PIX_FMT_BGR24, width, height);

            frame->width  = width;

            frame->height = height;

            //frame->data[0] = buffer;



            sws_scale(mImgConvertCtx, mFrameYuv->data, mFrameYuv->linesize,

                0, height, frame->data, frame->linesize);

        }



        return (AVFrame *)frame;
}

bool Decoder::decodeVideo(const AVPacket *avpkt, AVFrame *mOutFrame)
{
    bool res = false;

    if(mVideoCodecCtx)
    {
        if(avpkt && mOutFrame)
        {
            int gotPicPtr = 0;
            int videoFrameBytes = avcodec_decode_video2(mVideoCodecCtx,mOutFrame, &gotPicPtr, avpkt);

            res = (videoFrameBytes > 0);
        }
    }
    return res;
}

bool Decoder::openVideo()
{
    bool res = false;

    if(mFormatCtx)
    {
        videoStreamIndex = -1;

        for (unsigned int i = 0; i < mFormatCtx->nb_streams; i++)

                {

                    if (mFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)

                    {

                        videoStreamIndex = i;

                        mVideoCodecCtx = mFormatCtx->streams[i]->codec;

                        mVideoCodec = avcodec_find_decoder(mVideoCodecCtx->codec_id);



                        if (mVideoCodec)

                        {

                            res     = !(avcodec_open2(mVideoCodecCtx, mVideoCodec, NULL) < 0);

                            mWidth   = mVideoCodecCtx->coded_width;

                            mHeight  = mVideoCodecCtx->coded_height;

                        }



                        break;

                    }

                }
        if(!res)
        {
            closeVideo();
        }
        else
        {
                mImgConvertCtx = sws_getContext(mVideoCodecCtx->width, mVideoCodecCtx->height, mVideoCodecCtx->pix_fmt,
                                                mVideoCodecCtx->width, mVideoCodecCtx->height, AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
        }

    }
    return res;
}

void Decoder::closeVideo()
{
    if (mVideoCodecCtx)

        {

            avcodec_close(mVideoCodecCtx);

            mVideoCodecCtx = NULL;

            mVideoCodec = NULL;

            videoStreamIndex = 0;

        }
}
