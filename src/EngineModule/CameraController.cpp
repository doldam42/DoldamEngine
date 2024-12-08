#include "pch.h"

#include "GameManager.h"
#include "GameObject.h"

#include "Camera.h"
#include "CameraController.h"

void CameraController::Update(const float dt)
{
    if (m_pTarget && !m_useFirstPersonView)
    {
        float offset = 2.f;

        const Transform &targetTM = m_pTarget->GetTransform();

        Vector3 up = Vector3::UnitY;
        Vector3 viewDir = targetTM.GetForward();
        Vector3 eye = targetTM.GetPosition() + up * offset - viewDir * offset;

        m_pCamera->SetEyeAtUp(eye, targetTM.GetPosition(), up);
    }
    else
    {
        UpdateKeyboard(dt);

        float cursorNDCX = g_pGame->GetInputManager()->GetCursorNDCX();
        float cursorNDCY = g_pGame->GetInputManager()->GetCursorNDCY();

        if (m_prevCursorNDCX != cursorNDCX || m_prevcursorNDCY != cursorNDCY)
        {
            UpdateMouse(cursorNDCX, cursorNDCY);
            m_prevCursorNDCX = cursorNDCX;
            m_prevcursorNDCY = cursorNDCY;
        }
    }

    m_pCamera->Update();
}

void CameraController::UpdateKeyboard(const float dt)
{
    if (m_useFirstPersonView)
    {
        IInputManager *pInputManager = g_pGame->GetInputManager();

        float x = pInputManager->GetXAxis();
        float y = pInputManager->GetYAxis();
        float z = pInputManager->GetZAxis();

        if (x != 0 || y != 0 || z != 0)
        {
            Vector3 v(x, y, z);

            Vector3 forward = m_pCamera->GetForwardDir();
            Vector3 right = m_pCamera->GetRightDir();

            Vector3 pos = m_pCamera->GetPosition();

            Matrix m(right, Vector3::Up, forward);
            pos += Vector3::Transform(v, m) * m_speed * dt;

            m_pCamera->SetPosition(pos);
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
        
        m_pCamera->SetYawPitchRoll(yaw, -pitch, 0.0f);
    }
}

void CameraController::Cleanup()
{
    if (m_pCamera)
    {
        delete m_pCamera;
        m_pCamera = nullptr;
    }
}

void CameraController::MoveForward(float dt)
{
    Vector3 forward = m_pCamera->GetForwardDir();

    Vector3 pos = m_pCamera->GetPosition();
    m_pCamera->SetPosition(pos + forward * m_speed * dt);
}

void CameraController::MoveUp(float dt)
{
    Vector3 up = m_pCamera->GetUpDir();

    Vector3 pos = m_pCamera->GetPosition();
    m_pCamera->SetPosition(pos + up * m_speed * dt);
}

void CameraController::MoveRight(float dt)
{
    Vector3 right = m_pCamera->GetRightDir();

    Vector3 pos = m_pCamera->GetPosition();
    m_pCamera->SetPosition(pos + right * m_speed * dt);
}

void CameraController::SetAspectRatio(float aspect) { m_pCamera->SetAspectRatio(aspect); }

static void Jump(void *)
{
    float dt = g_pGame->GetDeltaTime();
    GameObject *pTarget = g_pGame->GetCamera()->GetFollowTarget();

    // pTarget->ApplyImpulseLinear(Vector3(0.0f, 2.0f, 0.0f));
}

void CameraController::SetFollowTarget(GameObject *pTarget)
{
    InputManager *pInputManager = (InputManager *)g_pGame->GetInputManager();
    pInputManager->AddKeyListener(VK_SPACE, Jump);
    m_useFirstPersonView = FALSE;
    m_pTarget = pTarget;
}

void CameraController::ToggleProjectionSetting()
{
    if (m_pCamera->m_usePerspectiveProjection)
    {
        m_pCamera->DisablePerspectiveProjection();
    }
    else
    {
        m_pCamera->EnablePerspectiveProjection();
    }
}

CameraController::CameraController() { m_pCamera = new Camera; }

CameraController::~CameraController() { Cleanup(); }

void CameraController::Initialize(float verticalFovRadians, float aspectRatio, float nearZ, float farZ)
{
    m_pCamera->Initialize(verticalFovRadians, aspectRatio, nearZ, farZ);

    InputManager *pInputManager = (InputManager *)g_pGame->GetInputManager();
    pInputManager->AddKeyListener(
        'G', [](void *) { g_pGame->GetCamera()->ToggleProjectionSetting(); }, nullptr);
    pInputManager->AddKeyListener(
        'F', [](void *) { g_pGame->ToggleCamera(); }, nullptr);
}
