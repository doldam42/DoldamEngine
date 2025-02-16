#include "pch.h"

#include "InputManager.h"

void InputManager::Initialize(UINT width, UINT height) { SetWindowSize(width, height); }

void InputManager::OnKeyDown(UINT nChar, UINT uiScanCode)
{
    m_keyPressed[nChar] = TRUE;

    m_eventListeners[nChar].Run();
}

void InputManager::OnKeyUp(UINT nChar, UINT uiScanCode) { m_keyPressed[nChar] = FALSE; }

BOOL InputManager::OnSysKeyDown(UINT nChar, UINT uiScanCode, BOOL bAltKeyDown) { return 0; }

void InputManager::OnMouseMove(int mouseX, int mouseY)
{
    // ���콺 Ŀ���� ��ġ�� NDC�� ��ȯ
    // ���콺 Ŀ���� ���� ��� (0, 0), ���� �ϴ�(width-1, height-1)
    // NDC�� ���� �ϴ��� (-1, -1), ���� ���(1, 1)
    m_cursorNdcX = mouseX * 2.0f / m_screenWidth - 1.0f;
    m_cursorNdcY = -mouseY * 2.0f / m_screenHeight + 1.0f;

    // Ŀ���� ȭ�� ������ ������ ��� ���� ����
    // ���ӿ����� Ŭ������ ���� ���� �ֽ��ϴ�.
    m_cursorNdcX = std::clamp(m_cursorNdcX, -1.0f, 1.0f);
    m_cursorNdcY = std::clamp(m_cursorNdcY, -1.0f, 1.0f);
}

void InputManager::OnMouseWheel(float wheelDelta)
{
    // TODO
    //m_lastWheelTime = GetTickCount64();
    /*if (m_isWheelStopped)
    {
        m_isWheelStopped = false;
    }

    m_deltaWheel = wheelDelta;*/
}

void InputManager::SetWindowSize(UINT width, UINT height)
{
    m_screenWidth = width;
    m_screenHeight = height;
}

BOOL InputManager::IsKeyPressed(UINT nChar) const
{
    assert(nChar < 256);
    return m_keyPressed[nChar];
}

GameEvent *InputManager::AddKeyListener(UINT nChar, const std::function<void(void *)> func, void *arg, size_t sizeOfArg)
{
    return m_eventListeners[nChar].AddCallback(func, arg, sizeOfArg);
}

void InputManager::DeleteKeyListener(UINT nChar, GameEvent *pEvent) { m_eventListeners[nChar].DeleteCallback(pEvent); }

float InputManager::GetXAxis() const
{
    if (m_keyPressed['A'])
        return -1.0;
    if (m_keyPressed['D'])
        return 1.0;
    return 0.0f;
}

float InputManager::GetYAxis() const
{
    if (m_keyPressed['Q'])
        return -1.0;
    if (m_keyPressed['E'])
        return 1.0;
    return 0.0f;
}

float InputManager::GetZAxis() const
{
    if (m_keyPressed['S'])
        return -1.0;
    if (m_keyPressed['W'])
        return 1.0;
    return 0.0f;
}

float InputManager::GetCursorNDCX() const { return m_cursorNdcX; }

float InputManager::GetCursorNDCY() const { return m_cursorNdcY; }

InputManager::~InputManager()
{
    for (int i = 0; i < 256; i++)
    {
        m_eventListeners[i].DeleteAllCallback();
    }
}
