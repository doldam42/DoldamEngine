#include "pch.h"

#include "Frustum.h"

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

        // view proj 행렬로부터 평면의 방정식과 절두체 생성
        Matrix m = m_viewProjMatrix.Transpose();

        // Left
        //Vector4 Left(m._14 + m._11, m._24 + m._21, m._34 + m._31, m._44 + m._41);
        //Vector4 Right(m._14 - m._11, m._24 - m._21, m._34 - m._31, m._44 - m._41);
        //Vector4 Bottom(m._14 + m._12, m._24 + m._22, m._34 + m._32, m._44 + m._42);
        //Vector4 Top(m._14 - m._12, m._24 - m._22, m._34 - m._32, m._44 - m._42);
        //Vector4 Near(m._13, m._23, m._33, m._44);
        //Vector4 Far(m._14 - m._13, m._24 - m._23, m._34 - m._33, m._44 - m._43);

        //std::array<Plane, 6> frustumPlanesFromMatrix = {
        //    Plane(Top), // up
        //    Plane(Bottom), // bottom
        //    Plane(Right), // right
        //    Plane(Left), // left
        //    Plane(Far), // far
        //    Plane(Near), // near
        //};

         Vector4 row1(m._11, m._12, m._13, m._14);
         Vector4 row2(m._21, m._22, m._23, m._24);
         Vector4 row3(m._31, m._32, m._33, m._34);
         Vector4 row4(m._41, m._42, m._43, m._44);

         std::array<Plane, 6> frustumPlanesFromMatrix = {
             Plane(-(row4 - row2)), // up
             Plane(-(row4 + row2)), // bottom
             Plane(-(row4 - row1)), // right
             Plane(-(row4 + row1)), // left
             Plane(-(row4 - row3)), // far
             Plane(-(row4 + row3)), // near
         };
        m_frustumWS = Frustum(frustumPlanesFromMatrix);
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

BOOL Camera::IsCulled(const Bounds &inBounds) const
{
    BoundCheckResult result = m_frustumWS.CheckBound(inBounds);
    return result == BoundCheckResult::Outside;
}
