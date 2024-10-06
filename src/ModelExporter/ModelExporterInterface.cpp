#include "pch.h"

#include "AssimpLoader.h"
#include "FBXLoader.h"

#include "ModelExporterInterface.h"

MODELEXPORTER_API BOOL CreateFbxExporter(IModelExporter **ppOutExporter)
{
    FBXLoader *pLoader = new FBXLoader;
    *ppOutExporter = pLoader;
    return TRUE;
}

MODELEXPORTER_API BOOL CreateAssimpExporter(IModelExporter **ppOutExporter)
{
    AssimpLoader *pLoader = new AssimpLoader;
    *ppOutExporter = pLoader;
    return TRUE;
}

MODELEXPORTER_API void DeleteFbxExporter(IModelExporter *pExporter) { delete dynamic_cast<FBXLoader *>(pExporter); }

MODELEXPORTER_API void DeleteAssimpExporter(IModelExporter *pExporter)
{
    delete dynamic_cast<AssimpLoader *>(pExporter);
}
