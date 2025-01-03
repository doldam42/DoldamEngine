#pragma once

class TimeController;
class BadAppleController;
class AudioManager;
class Client : public IController
{
  private:
    HWND            m_hWnd = nullptr;
    IGameManager   *m_pGame = nullptr;
    AudioManager   *m_pAudio = nullptr;

    // m_pGame에 종속된 객체 - m_pGame에서 delete한다.
    // TODO: Reference Count 추가
    IRenderer      *m_pRenderer = nullptr;
    IGameCharacter *m_pCharacter = nullptr;
    IGameObject    *m_pSphere = nullptr;

    IModelExporter *m_pFbxExporter = nullptr;
    IModelExporter *m_pAssimpExporter = nullptr;

    // TMP
    IFontHandle *m_pFontHandle = nullptr;

    UINT m_textImageWidth = 0;
    UINT m_textImageHeight = 0;

    BYTE *m_pTextImage = nullptr;
    BYTE *m_pImage = nullptr;

    IGameSprite *m_pStaticSprite = nullptr;
    IGameSprite *m_pDynamicSprite = nullptr;
    IGameSprite *m_pTextSprite = nullptr;

    ITextureHandle *m_pDynamicTexture = nullptr;

    WCHAR m_text[260] = {0};

    // Controllers
    TimeController* m_pTimeController;
    BadAppleController* m_pDemoController;

  private:
    void CleanupControllers();
    void Cleanup();

  public:
    BOOL Initialize(HWND hWnd);

    void LoadResources();

    void LoadScene();

    void Process();

    BOOL Start() override;
    void Update(float dt) override;

    void OnKeyDown(UINT nChar, UINT uiScanCode);
    void OnKeyUp(UINT nChar, UINT uiScanCode);
    void OnMouseMove(int mouseX, int mouseY);
    void OnMouseWheel(float deltaWheel);
    BOOL OnUpdateWindowSize(UINT width, UINT height);

    IGameManager *GetGameManager() { return m_pGame; }
    AudioManager *GetAudioManager() { return m_pAudio; }

    Client() = default;
    ~Client();
};

extern Client *g_pClient;