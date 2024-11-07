#include "pch.h"

#include "Camera.h"

void Camera::UpdateProjMatrix()

{
    m_projMatrix = m_usePerspectiveProjection
                       ? XMMatrixPerspectiveFovLH(m_verticalFOV, m_aspect, m_nearZ, m_farZ)
                       : XMMatrixOrthographicOffCenterLH(-m_aspect, m_aspect, -1.0f, 1.0f, m_nearZ, m_farZ);
}

void Camera::UpdateViewMatrix()
{
    m_viewMatrix = XMMatrixLookToLH(m_position, m_forwardDir, m_upDir);
    // Vector4 pos(m_position);
    // pos.w = 1.0f;
    // m_viewMatrix = Matrix(Vector4(m_rightDir), Vector4(m_upDir), Vector4(m_forwardDir), pos).Invert();
}

void Camera::Initialize(float verticalFovRadians, float aspectRatio, float nearZ, float farZ)
{
    SetPerspective(verticalFovRadians, aspectRatio, nearZ, farZ);

    m_isUpdated = TRUE;
}

void Camera::Update()
{
    if (m_isUpdated)
    {
        UpdateViewMatrix();

        m_prevViewProjMatrix = m_viewProjMatrix;
        m_viewProjMatrix = m_viewMatrix * m_projMatrix;
    }
    m_isUpdated = FALSE;
}

void Camera::SetEyeAtUp(Vector3 eye, Vector3 lookAt, Vector3 up)
{
    SetLookDirection(lookAt - eye, up);
    SetPosition(eye);

    m_isUpdated = TRUE;
}

void Camera::SetLookDirection(Vector3 forward, Vector3 up)
{
    forward.Normalize();
    up.Normalize();

    Vector3 right = -forward.Cross(up);

    m_forwardDir = forward;
    m_upDir = up;
    m_rightDir = right;

    m_isUpdated = TRUE;
}

void Camera::SetPosition(Vector3 worldPos)
{
    m_position = worldPos;
    m_isUpdated = TRUE;
}

void Camera::SetYawPitchRoll(float yaw, float pitch, float roll)
{
    Matrix R = Matrix::CreateFromYawPitchRoll(yaw, pitch, roll);
    m_forwardDir = Vector3::TransformNormal(Vector3::UnitZ, R);
    m_rightDir = Vector3::TransformNormal(Vector3::Right, R);
    m_upDir = Vector3::TransformNormal(Vector3::Up, R);

    m_isUpdated = TRUE;
}

void Camera::SetPerspective(float verticalFovRadians, float aspectRatio, float nearZ, float farZ)
{
    m_verticalFOV = verticalFovRadians;
    m_aspect = aspectRatio;
    m_nearZ = nearZ;
    m_farZ = farZ;

    UpdateProjMatrix();

    m_prevViewProjMatrix = m_viewProjMatrix;

    m_isUpdated = TRUE;
}

void Camera::SetFOV(float verticalFovRadians)
{
    m_verticalFOV = verticalFovRadians;
    UpdateProjMatrix();

    m_isUpdated = TRUE;
}

void Camera::SetAspectRatio(float aspectRatio)
{
    m_aspect = aspectRatio;
    UpdateProjMatrix();

    m_isUpdated = TRUE;
}

void Camera::SetZRange(float nearZ, float farZ)
{
    m_nearZ = nearZ;
    m_farZ = farZ;

    UpdateProjMatrix();

    m_isUpdated = TRUE;
}

void Camera::EnablePerspectiveProjection()
{
    m_usePerspectiveProjection = TRUE;
    UpdateProjMatrix();

    m_isUpdated = TRUE;
}

void Camera::DisablePerspectiveProjection()
{
    m_usePerspectiveProjection = FALSE;
    UpdateProjMatrix();

    m_isUpdated = TRUE;
}
