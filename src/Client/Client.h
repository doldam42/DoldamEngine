#pragma once

class CameraController;
class TimeController;
class BadAppleController;
class RaytracingDemoController;
class TessellationDemoController;
class InputManager;
class AudioManager;
class Client
{
  private:
    HMODULE m_hRendererDLL = nullptr;
    HMODULE m_hEngineDLL = nullptr;
    HMODULE m_hModelExporterDLL = nullptr;

    HWND          m_hWnd = nullptr;
    IGameManager *m_pGame = nullptr;
    AudioManager *m_pAudio = nullptr;

    IRenderer      *m_pRenderer = nullptr;
    IGameCharacter *m_pCharacter = nullptr;
    IGameObject    *m_pSphere = nullptr;

    IModelExporter *m_pFbxExporter = nullptr;
    IModelExporter *m_pAssimpExporter = nullptr;

    InputManager *m_pInputManager = nullptr;

    // Controllers
    TimeController             *m_pTimeController = nullptr;
    CameraController           *m_pCameraController = nullptr;
    BadAppleController         *m_pDemoController = nullptr;
    RaytracingDemoController   *m_pRaytracingDemoController = nullptr;
    TessellationDemoController *m_pTessellationDemoController = nullptr;

  private:
    BOOL LoadModules(HWND hWnd);

    void ProcessInput();

    void CleanupControllers();
    void Cleanup();

  public:
    BOOL Initialize(HWND hWnd);

    void LoadResources();

    void LoadScene();

    void Process();

    BOOL Start();
    // void Update(float dt);

    void OnKeyDown(UINT nChar, UINT uiScanCode);
    void OnKeyUp(UINT nChar, UINT uiScanCode);
    void OnMouseMove(int mouseX, int mouseY);
    void OnMouseWheel(float deltaWheel);
    BOOL OnUpdateWindowSize(UINT width, UINT height);

    IGameManager *GetGameManager() { return m_pGame; }
    AudioManager *GetAudioManager() { return m_pAudio; }

    IModelExporter *GetFBXModelExporter() { return m_pFbxExporter; }
    IModelExporter *GetAssimpExporter() { return m_pAssimpExporter; }

    InputManager *GetInputManager() { return m_pInputManager; }

    Client() = default;
    ~Client();
};

extern Client *g_pClient;