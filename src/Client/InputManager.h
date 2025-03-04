#pragma once

class InputManager
{
    float m_cursorNdcX = 0;
    float m_cursorNdcY = 0;

    UINT m_screenWidth = 0;
    UINT m_screenHeight = 0;

    bool m_prevKeyPressed[256] = {false};
    bool m_keyPressed[256] = {false};

    bool  m_isWheelStopped = true;
    float m_lastWheelTime = 0.0f;
    float m_deltaWheel = 0.0f;

  public:
    void Initialize(UINT width, UINT height);

    void ProcessInput();

    // Input ó��
    void OnKeyDown(UINT nChar, UINT uiScanCode);
    void OnKeyUp(UINT nChar, UINT uiScanCode);
    BOOL OnSysKeyDown(UINT nChar, UINT uiScanCode, BOOL bAltKeyDown);
    void OnMouseMove(int mouseX, int mouseY);
    void OnMouseWheel(float wheelDelta);
    void OnMouseLButtonDown();
    void OnMouseLButtonUp();
    void OnMouseRButtonDown();
    void OnMouseRButtonUp();

    void SetWindowSize(UINT width, UINT height);

    // repeat(false) : ���� Ű �Է� �ÿ��� TRUE�� ��ȯ
    // repeat(true)  : Ű�� ������ �ִ� ���� TRUE�� ��ȯ
    BOOL IsKeyPressed(UINT nChar, bool repeat = true) const;

    float GetXAxis() const;
    float GetYAxis() const;
    float GetZAxis() const;

    float GetCursorNDCX() const;
    float GetCursorNDCY() const;

    void Update();

    InputManager() = default;
    ~InputManager();
};
