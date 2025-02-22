#pragma once
#include <Windows.h>

class CameraController;
class InputManager;
class GUIController;
class FileManager;
class GameEditor
{
  private:
    WCHAR m_assetPath[MAX_PATH] = {L'\0'};

    HMODULE m_hRendererDLL = nullptr;
    HMODULE m_hEngineDLL = nullptr;
    HMODULE m_hModelExporterDLL = nullptr;

    HWND m_hWnd = nullptr;

    IGameManager *m_pGame = nullptr;
    IRenderer *m_pRenderer = nullptr;

    CameraController *m_pCameraController = nullptr;
    GUIController *m_pGUIController = nullptr;

    IModelExporter *m_pFbxExporter = nullptr;
    IModelExporter *m_pAssimpExporter = nullptr;

    InputManager *m_pInputManager = nullptr;

    FileManager *m_pFileManager = nullptr;

    BOOL LoadModules(HWND hWnd);
    void CleanupModules();

    BOOL LoadConfigs();

    void ProcessInput();

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

    InputManager *GetInputManager() { return m_pInputManager; }

    FileManager *GetFileManager() { return m_pFileManager; }

    const WCHAR *GetAssetPath() { return m_assetPath; }

    IModelExporter *GetFBXModelExporter() { return m_pFbxExporter; }
    IModelExporter *GetAssimpExporter() { return m_pAssimpExporter; }

    GameEditor() = default;
    ~GameEditor();
};

extern GameEditor *g_pEditor;