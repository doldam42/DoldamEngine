#pragma once
#include <Windows.h>

class SceneViewerController;
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

    SceneViewerController *m_pSceneViewerController = nullptr;

    BOOL LoadModules(HWND hWnd);
    void CleanupModules();

    void Cleanup();

  public:
    BOOL Initialize(HWND hWnd);
    void Process();

    void OnKeyDown(UINT nChar, UINT uiScanCode);
    void OnKeyUp(UINT nChar, UINT uiScanCode);
    void OnMouseMove(int mouseX, int mouseY);
    void OnMouseWheel(float deltaWheel);
    BOOL OnUpdateWindowSize(UINT width, UINT height);

    LRESULT WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    IGameManager *GetGameManager() { return m_pGame; }

    GameEditor() = default;
    ~GameEditor();
};

extern GameEditor *g_pEditor;