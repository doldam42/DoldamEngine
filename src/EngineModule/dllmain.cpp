// dllmain.cpp : Defines the entry point for the DLL application.

#include "pch.h"
#include "GameManager.h"

#ifdef _DEBUG
#pragma comment(lib, "MathModule_x64_Debug.lib")
#pragma comment(lib, "GenericModule_x64_Debug.lib")
#else
#pragma comment(lib, "MathModule_x64_Release.lib")
#pragma comment(lib, "GenericModule_x64_Release.lib")
#endif // _DEBUG

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
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
    IGameManager *pGame = new GameManager;

    if (!pGame)
    {
        hr = E_OUTOFMEMORY;
        goto lb_return;
    }
    hr = S_OK;
    *ppv = pGame;
lb_return:
    return hr;
}