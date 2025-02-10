#pragma once

#ifdef RENDERER_RAYTRACING
#include "../RendererRaytracing/D3D12Renderer.h"
#elif defined(RENDERER_D3D12)
#include "../RendererD3D12/D3D12Renderer.h"
#endif

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
    DRAW_PASS_TYPE passType;
    HANDLE         hThread;
    HANDLE         hEventList[RENDER_THREAD_EVENT_TYPE_COUNT];
};

UINT WINAPI RenderThread(void *pArg);
