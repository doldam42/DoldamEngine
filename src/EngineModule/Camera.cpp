#include "pch.h"

#include "GameEngine.h"
#include "GameObject.h"

#include "Camera.h"

void Camera::Update(const float dt)
{

    if (m_pTarget && !m_useFirstPersonView)
    {
        const Transform& targetTM = m_pTarget->GetTransform();
        float      offset = 1.f;

        m_yaw = targetTM.GetYAxis().y + XM_PI;
        // 3��Ī �������� pitch, roll�� ����
        m_pitch = 0;
        m_viewDir = targetTM.GetForward();
        m_position = targetTM.GetPosition() + Vector3::UnitY * 0.75 * offset - m_viewDir * offset;

        m_rightDir = m_upDir.Cross(m_viewDir);
    }
    else
    {
        UpdateKeyboard(dt);

        float cursorNDCX = g_pGame->GetInputManager()->GetCursorNDCX();
        float cursorNDCY = g_pGame->GetInputManager()->GetCursorNDCY();

        if (m_prevCursorNDCX != cursorNDCX || m_prevcursorNDCY != cursorNDCY)
        {
            m_isUpdated = true;
            UpdateMouse(cursorNDCX, cursorNDCY);
            m_prevCursorNDCX = cursorNDCX;
            m_prevcursorNDCY = cursorNDCY;

            UpdateViewMatrix();
        }
    }
    UpdateFrustum();
}

void Camera::UpdateViewDir()
{
    // �̵��� �� ������ �Ǵ� ����/������ ���� ���
    m_viewDir = Vector3::Transform(Vector3(0.0f, 0.0f, 1.0f), Matrix::CreateRotationY(this->m_yaw));
    m_rightDir = m_upDir.Cross(m_viewDir);
}

void Camera::UpdateKeyboard(const float dt)
{
    if (m_useFirstPersonView)
    {
        InputManager *pInputManager = g_pGame->GetInputManager();

        float x = pInputManager->GetXAxis();
        float y = pInputManager->GetYAxis();
        float z = pInputManager->GetZAxis();

        if (x != 0 || y != 0 || z != 0)
        {
            m_isUpdated = true;

            Vector3 v(x, y, z);
            Matrix  m(m_rightDir, m_upDir, m_viewDir);
            m_position += Vector3::Transform(v, m) * m_speed * dt;
        }
    }
}

void Camera::UpdateMouse(float mouseNdcX, float mouseNdcY)
{
    if (m_useFirstPersonView)
    {
        // �󸶳� ȸ������ ���
        m_yaw = mouseNdcX * XM_2PI;      // �¿� 360��
        m_pitch = mouseNdcY * XM_PIDIV2; // �� �Ʒ� 90��

        UpdateViewDir();
    }
}

void Camera::UpdateProjMatrix()
{
    SetProjMatrix(XMMatrixPerspectiveFovLH(XMConvertToRadians(m_projFovAngleY), m_aspect, m_nearZ, m_farZ));
    SetViewProjMatrix(m_viewMatrix * m_projMatrix);
}

void Camera::UpdateViewMatrix()
{
    // SetViewMatrix(Matrix::CreateLookAt(m_position, m_viewDir, m_upDir));
    SetViewMatrix(Matrix::CreateRotationY(-this->m_yaw) * Matrix::CreateRotationX(this->m_pitch));
    SetViewProjMatrix(m_viewMatrix * m_projMatrix);
}

void Camera::UpdateFrustum()
{
    m_frustumVS = BoundingFrustum(GetProjRow());
 
    m_frustumWS = m_frustumVS;
    m_frustumWS.Origin = m_position;
    m_frustumWS.Orientation = Quaternion::CreateFromYawPitchRoll(m_yaw, m_pitch, 0);
}

void Camera::MoveForward(float dt)
{
    // �̵�����_��ġ = ����_��ġ + �̵����� * �ӵ� * �ð�����;
    m_position += m_viewDir * m_speed * dt;
}

void Camera::MoveUp(float dt)
{
    // �̵�����_��ġ = ����_��ġ + �̵����� * �ӵ� * �ð�����;
    m_position += m_upDir * m_speed * dt;
}

void Camera::MoveRight(float dt) { m_position += m_rightDir * m_speed * dt; }

void Camera::SetAspectRatio(float aspect)
{
    m_aspect = aspect;
    UpdateProjMatrix();
}

void Camera::SetFollowTarget(GameObject *pTarget) { m_pTarget = pTarget; }
