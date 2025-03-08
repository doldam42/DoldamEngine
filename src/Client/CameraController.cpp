#include "pch.h"

#include "Client.h"
#include "InputManager.h"

#include "Camera.h"
#include "CameraController.h"

void CameraController::Update(const float dt)
{
    InputManager *pI = g_pClient->GetInputManager();
    if (pI->IsKeyPressed('F', false))
    {
        m_isFreezed = !m_isFreezed;
    }
    if (m_isFreezed)
        return;

    float cursorNDCX = pI->GetCursorNDCX();
    float cursorNDCY = pI->GetCursorNDCY();

    if (m_prevCursorNDCX != cursorNDCX || m_prevcursorNDCY != cursorNDCY)
    {
        UpdateMouse(cursorNDCX, cursorNDCY);
        m_prevCursorNDCX = cursorNDCX;
        m_prevcursorNDCY = cursorNDCY;
    }

    if (m_pTarget && !m_useFirstPersonView)
    {
        Vector3 pos = m_pTarget->GetPosition() + m_thirdPersonViewOffset;

        m_pTarget->SetRotationY(m_yaw);
        m_pGame->SetCameraPosition(pos.x, pos.y, pos.z);
    }
    else
    {
        UpdateKeyboard(dt);
    }
}

void CameraController::UpdateKeyboard(const float dt)
{
    InputManager *pI = g_pClient->GetInputManager();

    float dx = pI->GetXAxis();
    float dy = pI->GetYAxis();
    float dz = pI->GetZAxis();

    if (dx != 0.0f || dy != 0.0f || dz != 0.0f)
    {
        Vector3 pos = m_pGame->GetCameraPos();
        Vector3 forward = m_pGame->GetCameraLookTo();
        Vector3 right = Vector3::Up.Cross(forward);

        pos += (right * dx + Vector3::Up * dy + forward * dz) * m_speed * dt;

        m_pGame->SetCameraPosition(pos.x, pos.y, pos.z);
    }
}

void CameraController::UpdateMouse(float mouseNdcX, float mouseNdcY)
{
    // 얼마나 회전할지 계산
    float yaw = mouseNdcX * XM_2PI;      // 좌우 360도
    float pitch = mouseNdcY * XM_PIDIV2; // 위 아래 90도

    m_pGame->SetCameraYawPitchRoll(yaw, -pitch, 0.0f);

    m_yaw = yaw;
}

void CameraController::Cleanup() {}

void CameraController::MoveForward(float dt)
{
    Vector3 forward = m_pGame->GetCameraLookTo();

    Vector3 pos = m_pGame->GetCameraPos();
    pos += forward * m_speed * dt;
    m_pGame->SetCameraPosition(pos.x, pos.y, pos.z);
}

void CameraController::MoveUp(float dt)
{
    Vector3 up = Vector3::Up;

    Vector3 pos = m_pGame->GetCameraPos();
    pos += up * m_speed * dt;
    m_pGame->SetCameraPosition(pos.x, pos.y, pos.z);
}

void CameraController::MoveRight(float dt)
{
    Vector3 forward = m_pGame->GetCameraLookTo();
    Vector3 right = Vector3::Up.Cross(forward);

    Vector3 pos = m_pGame->GetCameraPos();
    pos += right * m_speed * dt;
    m_pGame->SetCameraPosition(pos.x, pos.y, pos.z);
}

void CameraController::SetFollowTarget(IGameObject *pTarget, Vector3 thirdPersonViewOffset)
{
    m_useFirstPersonView = FALSE;
    m_pTarget = pTarget;
    m_thirdPersonViewOffset = thirdPersonViewOffset;
}

CameraController::CameraController() 
{ 
    m_pGame = g_pClient->GetGameManager();
}

CameraController::~CameraController() { Cleanup(); }

BOOL CameraController::Start()
{
    return TRUE;
}

