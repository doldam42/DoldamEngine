// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "PhysicsManager.h"

#ifdef _DEBUG
#pragma comment(lib, "BulletDynamics_vs2010_x64_debug.lib")
#pragma comment(lib, "BulletCollision_vs2010_x64_debug.lib")
#pragma comment(lib, "LinearMath_vs2010_x64_debug.lib")

#pragma comment(lib, "MathModule_x64_Debug.lib")
#pragma comment(lib, "GenericModule_x64_Debug.lib")

#else // _DEBUG

#pragma comment(lib, "BulletDynamics_vs2010_x64_release.lib")
#pragma comment(lib, "BulletCollision_vs2010_x64_release.lib")
#pragma comment(lib, "LinearMath_vs2010_x64_release.lib")

#pragma comment(lib, "MathModule_x64_Release.lib")
#pragma comment(lib, "GenericModule_x64_Release.lib")
#endif // _RELEASE

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
    HRESULT       hr;
    IPhysicsManager *pPhysics = new PhysicsManager;

    if (!pPhysics)
    {
        hr = E_OUTOFMEMORY;
        goto lb_return;
    }
    hr = S_OK;
    *ppv = pPhysics;
lb_return:
    return hr;
}
