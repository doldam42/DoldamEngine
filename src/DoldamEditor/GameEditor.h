#pragma once
#include <Windows.h>

class GUIController;
class GameEditor
{
  private:
    HMODULE m_hRendererDLL = nullptr;
    HMODULE m_hEngineDLL = nullptr;

    HWND m_hWnd = nullptr;

    IGameManager *m_pGame = nullptr;
    IRenderer *m_pRenderer = nullptr;

    GUIController *m_pGUIController = nullptr;

    BOOL LoadModules(HWND hWnd);
    void CleanupModules();

    void Cleanup();

  public:
    BOOL Initialize(HWND hWnd);
    void Process();

    LRESULT WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    GameEditor() = default;
    ~GameEditor();
};
