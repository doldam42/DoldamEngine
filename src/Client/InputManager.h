#pragma once

class InputManager
{
    float m_cursorNdcX = 0;
    float m_cursorNdcY = 0;

    UINT m_screenWidth = 0;
    UINT m_screenHeight = 0;

    bool m_keyPressed[256] = {false};
    
    bool  m_isWheelStopped = true;
    float m_lastWheelTime = 0.0f;
    float m_deltaWheel = 0.0f;

    EventListener m_eventListeners[256];

  public:
    void Initialize(UINT width, UINT height);

    // Input Ã³¸®
    void OnKeyDown(UINT nChar, UINT uiScanCode);
    void OnKeyUp(UINT nChar, UINT uiScanCode);
    BOOL OnSysKeyDown(UINT nChar, UINT uiScanCode, BOOL bAltKeyDown);
    void OnMouseMove(int mouseX, int mouseY);
    void OnMouseWheel(float wheelDelta);

    void SetWindowSize(UINT width, UINT height);

    BOOL IsKeyPressed(UINT nChar) const;
    
    GameEvent *AddKeyListener(UINT nChar, const std::function<void(void *)> func, void *arg = nullptr,
                              size_t sizeOfArg = 0);
    void       DeleteKeyListener(UINT nChar, GameEvent *pEvent);

    float GetXAxis() const;
    float GetYAxis() const;
    float GetZAxis() const;

    float GetCursorNDCX() const;
    float GetCursorNDCY() const;

    InputManager() = default;
    ~InputManager();
};
