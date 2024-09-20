#include "pch.h"

#include "GameEngine.h"
#include "InputManager.h"

void InputManager::OnKeyDown(UINT nChar, UINT uiScanCode) {

  m_keyPressed[nChar] = TRUE;

  // f키 일인칭 시점
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
  // 마우스 커서의 위치를 NDC로 변환
  // 마우스 커서는 좌측 상단 (0, 0), 우측 하단(width-1, height-1)
  // NDC는 좌측 하단이 (-1, -1), 우측 상단(1, 1)
  m_cursorNdcX = mouseX * 2.0f / m_screenWidth - 1.0f;
  m_cursorNdcY = -mouseY * 2.0f / m_screenHeight + 1.0f;

  // 커서가 화면 밖으로 나갔을 경우 범위 조절
  // 게임에서는 클램프를 안할 수도 있습니다.
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
