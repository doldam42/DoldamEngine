#include "pch.h"

#include "GameEngine.h"
#include "InputManager.h"

void InputManager::OnKeyDown(UINT nChar, UINT uiScanCode) {

  m_keyPressed[nChar] = TRUE;

  // fŰ ����Ī ����
  if (m_keyPressed['F']) {
    g_pGame->ToggleCamera();
  }
}

void InputManager::OnKeyUp(UINT nChar, UINT uiScanCode) {
  m_keyPressed[nChar] = FALSE;
}

BOOL InputManager::OnSysKeyDown(UINT nChar, UINT uiScanCode, BOOL bAltKeyDown) {
  return 0;
}

void InputManager::OnMouseMove(int mouseX, int mouseY) {
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

void InputManager::SetWindowSize(UINT width, UINT height) {
  m_screenWidth = width;
  m_screenHeight = height;
}

BOOL InputManager::IsKeyPressed(UINT nChar) const {
  assert(nChar < 256);
  return m_keyPressed[nChar];
}

float InputManager::GetXAxis() const {
  if (m_keyPressed['A'])
    return -1.0;
  if (m_keyPressed['D'])
    return 1.0;
  return 0.0f;
}

float InputManager::GetYAxis() const {
  if (m_keyPressed['Q'])
    return -1.0;
  if (m_keyPressed['E'])
    return 1.0;
  return 0.0f;
}

float InputManager::GetZAxis() const {
  if (m_keyPressed['S'])
    return -1.0;
  if (m_keyPressed['W'])
    return 1.0;
  return 0.0f;
}

float InputManager::GetCursorNDCX() const { return m_cursorNdcX; }

float InputManager::GetCursorNDCY() const { return m_cursorNdcY; }
