#include "decoder.hpp"

Decoder::Decoder()
{
    mFormatCtx = nullptr;
    mVideoCodec = nullptr;
    mVideoCodecCtx = nullptr;
    mImgConvertCtx = nullptr;
    videoStreamIndex = -1;
    isOpen = false;
    ffmpegTimeBase = 0.0;
    fps = 0.0;
}

bool Decoder::openFile(const QString &videoInPut)
{

    closeFile();

    //av_register_all();


    if(avformat_open_input(&mFormatCtx, videoInPut.toUtf8().constData(), nullptr, nullptr) != 0)
    {
        closeFile();
        return false;
    }
    if(avformat_find_stream_info(mFormatCtx, nullptr) < 0)
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
            mFormatCtx = nullptr;

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
    AVFrame * res = nullptr;



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
    AVFrame * frame = nullptr;

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

                            res     = !(avcodec_open2(mVideoCodecCtx, mVideoCodec, nullptr) < 0);

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
                                                mVideoCodecCtx->width, mVideoCodecCtx->height, AV_PIX_FMT_BGR24, SWS_BICUBIC, nullptr, nullptr, nullptr);
        }

    }
    return res;
}

void Decoder::closeVideo()
{
    if (mVideoCodecCtx)

        {

            avcodec_close(mVideoCodecCtx);

            mVideoCodecCtx = nullptr;

            mVideoCodec = nullptr;

            videoStreamIndex = 0;

        }
}



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

