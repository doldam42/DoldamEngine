#include "pch.h"

#include "D3D12Renderer.h"

#include "RendererInterface.h"

BOOL CreateD3D12Renderer(HWND hWnd, BOOL bEnableDebugLayer, BOOL bEnableGBV, IRenderer **ppRenderer)
{
    BOOL           result = FALSE;
    D3D12Renderer *pD3DRenderer = new D3D12Renderer;
    result = pD3DRenderer->Initialize(hWnd, bEnableDebugLayer, bEnableGBV);
    if (!result)
    {
        __debugbreak();
        goto lb_return;
    }

    *ppRenderer = pD3DRenderer;
    g_pRenderer = pD3DRenderer;

    result = TRUE;
lb_return:
    return result;
}

void DeleteD3D12Renderer(IRenderer *pRenderer)
{
    if (pRenderer)
    {
        D3D12Renderer *pRnd = dynamic_cast<D3D12Renderer *>(pRenderer);
        delete pRnd;
        g_pRenderer = nullptr;
    }
    
#ifdef _DEBUG
    _ASSERT(_CrtCheckMemory());
#endif
}
