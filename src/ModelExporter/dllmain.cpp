// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#pragma comment(lib, "assimp-vc143-mt.lib")
#pragma comment(lib, "libfbxsdk.lib")

#pragma comment(lib, "GenericModule.lib")
#pragma comment(lib, "MathModule.lib")
#pragma comment(lib, "EngineModule.lib")

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

