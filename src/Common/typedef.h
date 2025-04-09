#pragma once

#include <Windows.h>

#define DEFULAT_LOCALE_NAME L"ko-kr"

#define DLL_API extern "C" __declspec(dllexport) HRESULT __stdcall

HRESULT typedef(__stdcall *CREATE_INSTANCE_FUNC)(void *ppv);

#ifdef _DEBUG
#define DASSERT(expr) ((!!(expr)) ? (void)0 : __debugbreak())
#else
#define DASSERT(expr) ((void)0)
#endif