// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "D3D12Renderer.h"

#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "dxCompiler.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

#pragma comment(lib, "DirectXTK12.lib")

#pragma comment(lib, "MathModule.lib")
#pragma comment(lib, "GenericModule.lib")

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

DLL_API DllCreateInstance(void **ppv)
{
    HRESULT    hr;
    IRenderer *pRenderer = new D3D12Renderer;

    if (!pRenderer)
    {
        hr = E_OUTOFMEMORY;
        goto lb_return;
    }
    hr = S_OK;
    *ppv = pRenderer;
lb_return:
    return hr;
}