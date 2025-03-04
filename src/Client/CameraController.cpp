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
    if (m_pTarget && !m_useFirstPersonView)
    {
        float offset = 2.f;

        Vector3 up = Vector3::UnitY;
        Vector3 viewDir = m_pTarget->GetForward();
        Vector3 eye = m_pTarget->GetPosition() + up * offset - viewDir * offset;

        m_pGame->SetCameraEyeAtUp(eye, m_pTarget->GetPosition(), up);
    }
    else
    {
        UpdateKeyboard(dt);

        float cursorNDCX = g_pClient->GetInputManager()->GetCursorNDCX();
        float cursorNDCY = g_pClient->GetInputManager()->GetCursorNDCY();

        if (m_prevCursorNDCX != cursorNDCX || m_prevcursorNDCY != cursorNDCY)
        {
            UpdateMouse(cursorNDCX, cursorNDCY);
            m_prevCursorNDCX = cursorNDCX;
            m_prevcursorNDCY = cursorNDCY;
        }
    }
}

void CameraController::UpdateKeyboard(const float dt)
{
    if (m_useFirstPersonView)
    {
        InputManager *pInputManager = g_pClient->GetInputManager();

        float x = pInputManager->GetXAxis();
        float y = pInputManager->GetYAxis();
        float z = pInputManager->GetZAxis();

        if (x != 0 || y != 0 || z != 0)
        {
            Vector3 v(x, y, z);

            Vector3 forward = m_pGame->GetCameraLookTo();
            Vector3 right = -forward.Cross(Vector3::Up);

            Vector3 pos = m_pGame->GetCameraPos();

            Matrix m(right, Vector3::Up, forward);
            pos += Vector3::Transform(v, m) * m_speed * dt;

            m_pGame->SetCameraPosition(pos.x, pos.y, pos.z);
        }
    }
}

void CameraController::UpdateMouse(float mouseNdcX, float mouseNdcY)
{
    if (m_useFirstPersonView)
    {
        // 얼마나 회전할지 계산
        float yaw = mouseNdcX * XM_2PI;      // 좌우 360도
        float pitch = mouseNdcY * XM_PIDIV2; // 위 아래 90도

        m_pGame->SetCameraYawPitchRoll(yaw, -pitch, 0.0f);
    }
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

void CameraController::SetFollowTarget(IGameObject *pTarget)
{
    m_useFirstPersonView = FALSE;
    m_pTarget = pTarget;
}

CameraController::CameraController() {}

CameraController::~CameraController() { Cleanup(); }

BOOL CameraController::Start() 
{
    return TRUE;
}

void CameraController::Render() {}

void CameraController::Initialize(Client *pClient) { m_pGame = pClient->GetGameManager(); }
