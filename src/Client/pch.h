// DoldamEngine
#pragma once

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <windows.h>

#include "../Common/typedef.h"
#include "../MathModule/MathHeaders.h"
#include "../GenericModule/GenericHeaders.h"
#include "../Common/RendererInterface.h"
#include "../Common/EngineInterface.h"
#include "../Common/ModelExporterInterface.h"
