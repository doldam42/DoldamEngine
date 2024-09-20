#pragma once

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <windows.h>

#include "EngineHeaders.h"
#include "MathHeaders.h"
#include "GenericHeaders.h"
#include "RenderModule.h"

#include "EngineModule.h"