#pragma once

class TimeController;
class BadAppleController;
class RaytracingDemoController;
class TessellationDemoController;
class AudioManager;
class Client
{
  private:
    HWND            m_hWnd = nullptr;
    IGameManager   *m_pGame = nullptr;
    AudioManager   *m_pAudio = nullptr;

    // m_pGame�� ���ӵ� ��ü - m_pGame���� delete�Ѵ�.
    // TODO: Reference Count �߰�
    IRenderer      *m_pRenderer = nullptr;
    IGameCharacter *m_pCharacter = nullptr;
    IGameObject    *m_pSphere = nullptr;

    IModelExporter *m_pFbxExporter = nullptr;
    IModelExporter *m_pAssimpExporter = nullptr;

    // Controllers
    TimeController* m_pTimeController = nullptr;
    BadAppleController* m_pDemoController = nullptr;
    RaytracingDemoController *m_pRaytracingDemoController = nullptr;
    TessellationDemoController *m_pTessellationDemoController = nullptr;

  private:
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

    Client() = default;
    ~Client();
};

extern Client *g_pClient;