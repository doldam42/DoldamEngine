#pragma once

#ifdef MODELEXPORTER_EXPORTS
#define MODELEXPORTER_API __declspec(dllexport)
#else
#define MODELEXPORTER_API __declspec(dllimport)
#endif

#include <combaseapi.h>

interface IGameModel;
interface IGameAnimation;
interface IModelExporter : public IUnknown
{ 
	virtual BOOL Initialize(IGameManager* pGame) = 0;

	virtual BOOL Load(const WCHAR *basePath, const WCHAR *filename) = 0;
    virtual BOOL LoadAnimation(const WCHAR *filename) = 0;
	
	virtual IGameModel *GetModel() = 0;
    virtual IGameAnimation *GetAnimation() = 0;

	virtual void ExportModel() = 0;
    virtual void ExportAnimation() = 0;
};
