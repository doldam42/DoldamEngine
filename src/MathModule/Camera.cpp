#include "pch.h"

#include "Camera.h"

void Camera::UpdateProjMatrix() { SetProjMatrix(XMMatrixPerspectiveFovLH(m_verticalFOV, m_aspect, m_nearZ, m_farZ)); }

void Camera::Initialize(float verticalFovRadians, float aspectRatio, float nearZ, float farZ)
{
    SetPerspectiveMatrix(XM_PIDIV4, 9.0f / 16.0f, 1.0f, 1000.0f);
}

void Camera::Update() 
{ 
    m_prevViewProjMatrix = m_viewProjMatrix;
    m_viewMatrix = m_cameraToWorld.GetMatrix().Invert();
    m_viewProjMatrix = m_projMatrix * m_viewMatrix;

    m_frustumVS = BoundingFrustum(m_projMatrix);
    m_frustumVS.Transform(m_frustumWS, m_cameraToWorld.GetMatrix());
}

void Camera::SetEyeAtUp(Vector3 eye, Vector3 at, Vector3 up)
{
    SetLookDirection(at - eye, up);
    SetPosition(eye);
}

void Camera::SetLookDirection(Vector3 forward, Vector3 up)
{
    forward.Normalize();
    up.Normalize();

    Vector3 right = forward.Cross(up);
}

void Camera::SetRotation(Quaternion basisRotation)
{
    basisRotation.Normalize();
    m_cameraToWorld.SetRotation(basisRotation);
    Matrix m = Matrix::CreateFromQuaternion(basisRotation);
    m_rightDir = Vector3::Transform(Vector3::UnitX, m);
    m_upDir = Vector3::Transform(Vector3::UnitY, m);
    m_ForwardDir = Vector3::Transform(-Vector3::UnitZ, m);
}

void Camera::SetPosition(Vector3 worldPos) { m_cameraToWorld.SetPosition(worldPos); }

void Camera::SetTransform(const Transform &tm)
{
    SetLookDirection(-tm.GetZAxis(), tm.GetYAxis());
    SetPosition(tm.GetPosition());
}

void Camera::SetPerspectiveMatrix(float verticalFovRadians, float aspectRatio, float nearZ, float farZ)
{
    m_verticalFOV = verticalFovRadians;
    m_aspect = aspectRatio;
    m_nearZ = nearZ;
    m_farZ = farZ;

    UpdateProjMatrix();

    m_prevViewProjMatrix = m_viewProjMatrix;
}

void Camera::SetFOV(float verticalFovRadians) { m_verticalFOV = verticalFovRadians; }

void Camera::SetAspectRatio(float aspectRatio) { m_aspect = aspectRatio; }

void Camera::SetZRange(float nearZ, float farZ)
{
    m_nearZ = nearZ;
    m_farZ = farZ;
}
