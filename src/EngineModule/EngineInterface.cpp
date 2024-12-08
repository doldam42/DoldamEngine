#include "pch.h"

#include "GameManager.h"
#include "MeshObject.h"

#include "EngineInterface.h"

ENGINEMODULE_API BOOL CreateGameEngine(HWND hWnd, IGameManager **ppOutGameEngine)
{
    BOOL result = false;

    GameManager *pGameEngine = new GameManager;

    result = pGameEngine->Initialize(hWnd);
    *ppOutGameEngine = pGameEngine;

    g_pGame = pGameEngine;

    return result;
}

ENGINEMODULE_API void DeleteGameEngine(IGameManager *pGameEngine)
{
    GameManager *pGame = (GameManager *)pGameEngine;
    delete pGame;

#ifdef _DEBUG
    _ASSERT(_CrtCheckMemory());
#endif
}

ENGINEMODULE_API BOOL CreateGameMesh(IGameMesh **ppOutMesh)
{
    MeshObject *pMesh = new MeshObject;
    *ppOutMesh = pMesh;
    return TRUE;
}

ENGINEMODULE_API void DeleteGameMesh(IGameMesh *pInMesh) 
{
    MeshObject *pMesh = (MeshObject *)pInMesh;
    delete pMesh;
}
