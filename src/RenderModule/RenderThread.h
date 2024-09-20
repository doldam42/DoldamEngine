#pragma once

#include "D3D12Renderer.h"
enum RENDER_THREAD_EVENT_TYPE
{
	RENDER_THREAD_EVENT_TYPE_PROCESS = 0,
    RENDER_THREAD_EVENT_TYPE_DESTROY,
    RENDER_THREAD_EVENT_TYPE_COUNT,
};
struct RENDER_THREAD_DESC
{
    D3D12Renderer *pRenderer;
    UINT           threadindex;
    HANDLE         hThread;
    HANDLE         hEventList[RENDER_THREAD_EVENT_TYPE_COUNT];
};

UINT WINAPI RenderThread(void *pArg);
