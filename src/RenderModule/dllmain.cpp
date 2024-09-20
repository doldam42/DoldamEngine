// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "dxCompiler.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

#pragma comment(lib, "DirectXTK12.lib")

#pragma comment(lib, "GenericModule.lib")
#pragma comment(lib, "MathModule.lib")

//////////////////////////////////////////////////////////////////////////////////////////////////////
// D3D12 Agility SDK Runtime

extern "C" {
__declspec(dllexport) extern const UINT D3D12SDKVersion = 614;
}

#if defined(_M_ARM64EC)
extern "C" {
__declspec(dllexport) extern const char *D3D12SDKPath = u8".\\D3D12\\arm64\\";
}
#elif defined(_M_ARM64)
extern "C" {
__declspec(dllexport) extern const char *D3D12SDKPath = u8".\\D3D12\\arm64\\";
}
#elif defined(_M_AMD64)
extern "C" {
__declspec(dllexport) extern const char *D3D12SDKPath = u8".\\D3D12\\x64\\";
}
#elif defined(_M_IX86)
extern "C" {
__declspec(dllexport) extern const char *D3D12SDKPath = u8".\\D3D12\\x86\\";
}
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////

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

