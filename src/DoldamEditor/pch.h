#pragma once

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#define DEBUG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
// C RunTime Header Files
#include <malloc.h>
#include <memory.h>
#include <stdlib.h>
#include <tchar.h>
