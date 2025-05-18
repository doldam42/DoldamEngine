// RendererModule
// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

// add headers that you want to pre-compile here
#include "framework.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3d11on12.h>
#include <dwrite_3.h>
#include <d2d1_3.h>
#include <d3dx12.h>
#include <dxgidebug.h>
#include <dxcapi.h>

#include "typedef.h"
#include "../MathModule/MathHeaders.h"
#include "../GenericModule/GenericHeaders.h"

#include "D3DUtil.h"

#endif //PCH_H
