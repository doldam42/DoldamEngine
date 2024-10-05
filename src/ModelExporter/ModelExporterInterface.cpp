#include "pch.h"

#include "FBXLoader.h"

#include "ModelExporterInterface.h"

MODELEXPORTER_API BOOL CreateFbxExporter(IModelExporter **ppOutExporter) 
{ 
    FBXLoader *pLoader = new FBXLoader;
    *ppOutExporter = pLoader;
    return TRUE;
}

MODELEXPORTER_API BOOL CreateGltfExporter(IModelExporter **ppOutExporter) { return FALSE; }

MODELEXPORTER_API void DeleteModelExporter(IModelExporter *pExporter) 
{
    delete (FBXLoader*)pExporter;

#ifdef _DEBUG
    _ASSERT(_CrtCheckMemory());
#endif
}
