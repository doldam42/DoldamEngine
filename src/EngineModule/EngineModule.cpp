#include "pch.h"

#include "GameEngine.h"

#include "EngineModule.h"

ENGINEMODULE_API BOOL CreateGameEngine(HWND hWnd, IGameEngine **ppOutGameEngine)
{
    BOOL result = false;

    GameEngine *pGameEngine = new GameEngine;

    result = pGameEngine->Initialize(hWnd);
    *ppOutGameEngine = pGameEngine;

    g_pGame = pGameEngine;

    return result;
}

ENGINEMODULE_API void DeleteGameEngine(IGameEngine *pGameEngine)
{
    GameEngine *pGame = (GameEngine *)pGameEngine;
    delete pGame;

#ifdef _DEBUG
    _ASSERT(_CrtCheckMemory());
#endif
}
