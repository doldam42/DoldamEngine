#include "pch.h"
#include "VideoManager.h"

BOOL CreateVideoHandle(VIDEO_HANDLE **ppOutVideo, const WCHAR *wpath)
{
    char path[MAX_PATH] = {'\0'};
    ws2s(wpath, path);

    AVFormatContext *pFormatContext = avformat_alloc_context();
    if (!pFormatContext)
    {
#ifdef _DEBUG
        __debugbreak();
#endif // _DEBUG
        return FALSE;
    }
    if (avformat_open_input(&pFormatContext, path, NULL, NULL) != 0)
    {
#ifdef _DEBUG
        __debugbreak();
#endif // _DEBUG
        return FALSE;
    }

    // 비디오 스트림 찾기
    if (avformat_find_stream_info(pFormatContext, nullptr) < 0)
    {
#ifdef _DEBUG
        __debugbreak();
#endif // _DEBUG
        return FALSE;
    }

    int videoStreamIndex = -1;
    for (unsigned int i = 0; i < pFormatContext->nb_streams; i++)
    {
        if (pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex = i;
            break;
        }
    }
    if (videoStreamIndex == -1)
    {
#ifdef _DEBUG
        __debugbreak();
#endif // _DEBUG
        return FALSE;
    }

    // 3. 코덱 초기화
    AVCodecParameters *codecParams = pFormatContext->streams[videoStreamIndex]->codecpar;
    const AVCodec     *codec = avcodec_find_decoder(codecParams->codec_id);
    if (!codec)
    {
#ifdef _DEBUG
        __debugbreak();
#endif // _DEBUG
        return FALSE;
    }

    AVCodecContext *codecContext = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecContext, codecParams);
    if (avcodec_open2(codecContext, codec, nullptr) < 0)
    {
#ifdef _DEBUG
        __debugbreak();
#endif // _DEBUG
        return FALSE;
    }

    // 패킷 및 프레임 할당
    AVPacket *packet = av_packet_alloc();
    AVFrame  *frame = av_frame_alloc();

    int width = codecContext->width;
    int height = codecContext->height;

    // SWS Context 생성 (픽셀 포맷 변환 준비)
    struct SwsContext *swsContext = sws_getContext(width, height, codecContext->pix_fmt, // 소스 포맷 (예: YUV420P)
                                                   width, height, AV_PIX_FMT_RGBA,       // 타겟 포맷 (예: RGBA)
                                                   SWS_BILINEAR, nullptr, nullptr, nullptr);

    if (!swsContext)
    {
#ifdef _DEBUG
        __debugbreak();
#endif // _DEBUG
        return FALSE;
    }

    // RGBA frame 생성
    int   num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, width, height, 1);
    BYTE *buffer = (BYTE *)av_malloc(num_bytes * sizeof(BYTE));
    memset(buffer, 0, num_bytes * sizeof(BYTE));

    AVRational timeBase = pFormatContext->streams[videoStreamIndex]->time_base;

    VIDEO_HANDLE *pVideo = new VIDEO_HANDLE;
    pVideo->pFormatContext = pFormatContext;
    pVideo->pCodecContext = codecContext;
    pVideo->swsContext = swsContext;

    pVideo->pPacket = packet;
    pVideo->pFrame = frame;
    pVideo->pRGBAImage = buffer;

    pVideo->width = width;
    pVideo->height = height;

    pVideo->videoStreamIndex = videoStreamIndex;

    pVideo->elapsedTime = 0.0f;
    pVideo->timeBase = timeBase;
    *ppOutVideo = pVideo;

    return TRUE;
}

void DeleteVideoHandle(VIDEO_HANDLE *pVideo)
{
    if (pVideo->pRGBAImage)
    {
        av_free(pVideo->pRGBAImage);
        pVideo->pRGBAImage = nullptr;
    }
    if (pVideo->pFrame)
    {
        av_frame_free(&pVideo->pFrame);
        pVideo->pFrame = nullptr;
    }
    if (pVideo->pPacket)
    {
        av_packet_free(&pVideo->pPacket);
        pVideo->pPacket = nullptr;
    }
    if (pVideo->swsContext)
    {
        sws_freeContext(pVideo->swsContext);
        pVideo->swsContext = nullptr;
    }
    if (pVideo->pCodecContext)
    {
        avcodec_free_context(&pVideo->pCodecContext);
        pVideo->pCodecContext = nullptr;
    }
    if (pVideo->pFormatContext)
    {
        avformat_close_input(&pVideo->pFormatContext);
        pVideo->pFormatContext = nullptr;
    }

    delete pVideo;
}

BOOL VideoPlay(VIDEO_HANDLE *pVideo, float dt)
{
    AVFormatContext *pFormatContext = pVideo->pFormatContext;
    AVCodecContext  *pCodecContext = pVideo->pCodecContext;
    SwsContext      *swsContext = pVideo->swsContext;

    AVPacket *pPacket = pVideo->pPacket;
    AVFrame  *pFrame = pVideo->pFrame;

    AVRational timeBase = pVideo->timeBase;

    int videoStreamIndex = pVideo->videoStreamIndex;
    int response = pVideo->response;

    int width = pVideo->width;
    int height = pVideo->height;
    
    int linesize = av_image_get_linesize(AV_PIX_FMT_RGBA, width, 0);
    
    uint8_t *rgba[1] = {pVideo->pRGBAImage};
    int rgba_stride[1] = {linesize};

    // 현재 재생 시간 계산
    pVideo->elapsedTime += dt;
    float elapsedTime = pVideo->elapsedTime;

    // 디코딩 루프
    while (av_read_frame(pFormatContext, pPacket) >= 0)
    {
        if (pPacket->stream_index == videoStreamIndex)
        {
            int response = avcodec_send_packet(pCodecContext, pPacket);
            if (response < 0)
            {
#ifdef _DEBUG
                __debugbreak();
#endif // _DEBUG
                return FALSE;
            }

            while (response >= 0)
            {
                response = avcodec_receive_frame(pCodecContext, pFrame);
                if (response == AVERROR(EAGAIN) || response == AVERROR_EOF)
                {
                    break;
                }
                else if (response < 0)
                {
#ifdef _DEBUG
                    __debugbreak();
#endif // _DEBUG
                    return FALSE;
                }

                // 프레임의 PTS를 초 단위로 변환
                float frameTime = pFrame->pts * av_q2d(timeBase);

                if (frameTime < elapsedTime)
                {
                    // 해당 프레임 건너뛰기
                    goto lb_continue;
                }

                // 프레임 처리
                sws_scale(swsContext, pFrame->data, pFrame->linesize, 0, height, rgba, rgba_stride);

                return TRUE;
                //if (frameTime > elapsedTime)
                //{
                //    // 프레임 표시까지 대기
                //    return TRUE;
                //}

                // 여기서 프레임 데이터를 메모리로 복사
                // int           width = pFrame->width;
                // int           height = pFrame->height;
                // AVPixelFormat format = (AVPixelFormat)pFrame->format;
                //// 프레임 데이터 메모리 할당
                // int      numBytes = av_image_get_buffer_size(format, width, height, 1);
                // uint8_t *frameBuffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
                // av_image_copy_to_buffer(frameBuffer, numBytes, pFrame->data, pFrame->linesize, format, width,
                //                         height, 1);
                //  메모리에 매핑된 데이터를 사용 (예: 렌더링, 저장 등)
                // save_gray_frame(pFrame->data[0], pFrame->linesize[0], width, height, "img.pgm");
                //  메모리 해제
                // av_free(frameBuffer);
            }
            return TRUE;
        }
    lb_continue:
        av_packet_unref(pPacket);
    }
}