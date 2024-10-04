#pragma once
class Client
{
    HWND         m_hWnd = nullptr;
    IGameEngine *m_pGame = nullptr;
    // m_pGame에 종속된 객체 - m_pGame에서 delete한다.
    IRenderer   *m_pRenderer = nullptr;

    // Timer
    UINT m_FPS = 0;
    UINT m_frameCount = 0;
    UINT m_prevFrameCheckTick = 0;

    ULONGLONG m_prevUpdateTick = 0;

    // TMP
    IFontHandle *m_pFontHandle = nullptr;

    UINT m_textImageWidth = 0;
    UINT m_textImageHeight = 0;

    BYTE *m_pTextImage = nullptr;
    BYTE *m_pImage = nullptr;

    IGameSprite *m_pStaticSprite = nullptr;
    IGameSprite *m_pDynamicSprite = nullptr;
    IGameSprite *m_pTextSprite = nullptr;

    WCHAR m_text[260] = {0};

  private:
    void Cleanup();

  public:
    BOOL Initialize(HWND hWnd);

    void LoadResources();

    void Process();

    void Update(ULONGLONG curTick);

    void OnKeyDown(UINT nChar, UINT uiScanCode);
    void OnKeyUp(UINT nChar, UINT uiScanCode);
    void OnMouseMove(int mouseX, int mouseY);
    BOOL OnUpdateWindowSize(UINT width, UINT height);

    Client() = default;
    ~Client();
};
