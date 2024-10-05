#pragma once
#include "EngineInterface.h"

class InputManager : public IInputManager
{
    float m_cursorNdcX = 0;
    float m_cursorNdcY = 0;

    UINT m_screenWidth = 0;
    UINT m_screenHeight = 0;

    bool m_keyPressed[256] = {false};

  public:
    // Input Ã³¸®
    void OnKeyDown(UINT nChar, UINT uiScanCode);
    void OnKeyUp(UINT nChar, UINT uiScanCode);
    BOOL OnSysKeyDown(UINT nChar, UINT uiScanCode, BOOL bAltKeyDown);
    void OnMouseMove(int mouseX, int mouseY);

    void SetWindowSize(UINT width, UINT height);

    BOOL IsKeyPressed(UINT nChar) const;

    float GetXAxis() const;
    float GetYAxis() const;
    float GetZAxis() const;

    float GetCursorNDCX() const;
    float GetCursorNDCY() const;

    InputManager() = default;
    ~InputManager() = default;
};
