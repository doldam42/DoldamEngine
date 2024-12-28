#pragma once

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

struct VIDEO_HANDLE
{
    AVFormatContext *pFormatContext = nullptr;
    AVCodecContext  *pCodecContext = nullptr;
    SwsContext      *swsContext = nullptr;

    AVPacket *pPacket = nullptr;
    AVFrame  *pFrame = nullptr;
    BYTE     *pRGBAImage = nullptr;

    AVRational timeBase;

    float elapsedTime = 0;

    int videoStreamIndex = 0;
    int response = 0;

    int width = 0;
    int height = 0;

    BOOL isUpdated = FALSE;
};

BOOL CreateVideoHandle(VIDEO_HANDLE **ppOutVideo, const WCHAR *wpath);
void DeleteVideoHandle(VIDEO_HANDLE *pVideo);
BOOL VideoPlay(VIDEO_HANDLE *pVideo, float dt);