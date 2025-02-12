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

// For Windows File Dialog
#define STRICT_TYPED_ITEMIDS
#include <knownfolders.h> // for KnownFolder APIs/datatypes/function headers
#include <new>
#include <objbase.h>     // For COM headers
#include <propidl.h>     // for the Property System APIs
#include <propkey.h>     // for the Property key APIs/datatypes
#include <propvarutil.h> // for PROPVAR-related functions
#include <shlobj.h>
#include <shlwapi.h>
#include <shobjidl.h> // for IFileDialogEvents and IFileDialogControlEvents
#include <shtypes.h>  // for COMDLG_FILTERSPEC
#include <strsafe.h>  // for StringCchPrintfW

#include "../Common/typedef.h"
#include "../Common/RendererInterface.h"
#include "../Common/EngineInterface.h"