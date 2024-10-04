#pragma once

#ifdef MODELEXPORTER_EXPORTS
#define MODELEXPORTER_API __declspec(dllexport)
#else
#define MODELEXPORTER_API __declspec(dllimport)
#endif

class IGameModel;
class IGameAnimation;
interface IModelExporter 
{ 
	virtual BOOL Initialize(IGameEngine* pGame) = 0;

	virtual BOOL Load(const WCHAR *basePath, const WCHAR *filename) = 0;
    virtual BOOL LoadAnimation(const WCHAR *filename) = 0;

	virtual IGameModel *GetModel() = 0;
    virtual IGameAnimation *GetAnimation() = 0;
};

extern "C" MODELEXPORTER_API BOOL CreateFbxExporter(IModelExporter **ppOutExporter);

extern "C" MODELEXPORTER_API BOOL CreateGltfExporter(IModelExporter **ppOutExporter);

extern "C" MODELEXPORTER_API void DeleteModelExporter(IModelExporter *pExporter);
