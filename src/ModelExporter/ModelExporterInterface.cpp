#include "pch.h"

#include "ModelExporterInterface.h"

MODELEXPORTER_API BOOL CreateFbxExporter(IModelExporter **ppOutExporter) { return FALSE; }

MODELEXPORTER_API BOOL CreateGltfExporter(IModelExporter **ppOutExporter) { }

MODELEXPORTER_API void DeleteModelExporter(IModelExporter *pExporter) 
{
#ifdef _DEBUG
    _ASSERT(_CrtCheckMemory());
#endif
}
