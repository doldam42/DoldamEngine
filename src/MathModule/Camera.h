#pragma once

class Camera
{
    float m_verticalFOV = 90.0f;
    float m_nearZ = 0.01f;
    float m_farZ = 100.0f;
    float m_aspect = 16.0f / 9.0f;
    bool  m_usePerspectiveProjection = true;

    Transform m_cameraToWorld;

    Vector3 m_rightDir;
    Vector3 m_upDir;
    Vector3 m_ForwardDir;

    Matrix m_viewMatrix;
    Matrix m_projMatrix;
    Matrix m_viewProjMatrix;
    Matrix m_prevViewProjMatrix;

    BoundingFrustum m_frustumVS; // View-space view frustum
    BoundingFrustum m_frustumWS; // World-space view frustum

  private:
    void UpdateProjMatrix();

  public:
    void Initialize(float verticalFovRadians, float aspectRatio, float nearZ, float farZ);

    void Update();

    void SetEyeAtUp(Vector3 eye, Vector3 at, Vector3 up);
    void SetLookDirection(Vector3 forward, Vector3 up);
    void SetRotation(Quaternion basisRotation);
    void SetPosition(Vector3 worldPos);
    void SetTransform(const Transform &tm);

    // Controls the view-to-projection matrix
    void SetPerspectiveMatrix(float verticalFovRadians, float aspectRatio, float nearZ, float farZ);
    void SetFOV(float verticalFovRadians);
    void SetAspectRatio(float aspectRatio);
    void SetZRange(float nearZ, float farZ);

    void SetProjMatrix(const Matrix &projMat) { m_projMatrix = projMat; }

    const Quaternion GetRotation() const { return m_cameraToWorld.GetRotation(); }
    const Vector3    GetRightDir() const { return m_rightDir; }
    const Vector3    GetUpDir() const { return m_upDir; }
    const Vector3    GetForwardDir() const { return m_ForwardDir; }
    const Vector3    GetPosition() const { return m_cameraToWorld.GetPosition(); }

    const Matrix          &GetViewMatrix() const { return m_viewMatrix; }
    const Matrix          &GetProjMatrix() const { return m_projMatrix; }
    const Matrix          &GetViewProjMatrix() const { return m_viewProjMatrix; }
    const BoundingFrustum &GetViewSpaceFrustum() const { return m_frustumVS; }
    const BoundingFrustum &GetWorldSpaceFrustum() const { return m_frustumWS; }
};