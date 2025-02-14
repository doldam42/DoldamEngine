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

#ifdef _DEBUG
#pragma comment(lib, "../../../lib/Debug/DirectXTex.lib")
#pragma comment(lib, "../../../DLL/MathModule_x64_Debug.lib")
#pragma comment(lib, "../../../DLL/GenericModule_x64_Debug.lib")
#else
#pragma comment(lib, "../../../lib/Release/DirectXTex.lib")
#pragma comment(lib, "../../../DLL/MathModule_x64_Release.lib")
#pragma comment(lib, "../../../DLL/GenericModule_x64_Release.lib")
#endif // _DEBUG


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