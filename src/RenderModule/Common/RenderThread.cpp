#include "pch.h"

#ifdef RENDERER_RAYTRACING
#include "../RendererRaytracing/D3D12Renderer.h"
#elif defined(RENDERER_D3D12)
#include "../RendererD3D12/D3D12Renderer.h"
#endif

#include <Windows.h>
#include <process.h>

#include "RenderThread.h"

UINT __stdcall RenderThread(void *pArg)
{
    RENDER_THREAD_DESC *pDesc = (RENDER_THREAD_DESC *)pArg;
    D3D12Renderer      *pRenderer = pDesc->pRenderer;
    UINT                threadIndex = pDesc->threadindex;
    DRAW_PASS_TYPE      passType = pDesc->passType;
    const HANDLE       *phEventList = pDesc->hEventList;
    while (1)
    {
        UINT eventIndex = WaitForMultipleObjects(RENDER_THREAD_EVENT_TYPE_COUNT, phEventList, FALSE, INFINITE);

        switch (eventIndex)
        {
        case RENDER_THREAD_EVENT_TYPE_PROCESS:
            pRenderer->ProcessByThread(threadIndex, passType);
            break;
        case RENDER_THREAD_EVENT_TYPE_DESTROY:
            goto lb_exit;
        default:
            __debugbreak();
        }
    }
lb_exit:
    _endthreadex(0);
    return 0;
}
