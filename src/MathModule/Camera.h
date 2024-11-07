#pragma once

// Camera의 모든 각도는 Radian으로 계산
class Camera
{
    float m_verticalFOV = XMConvertToRadians(120);
    float m_nearZ = 0.01f;
    float m_farZ = 100.0f;
    float m_aspect = 16.0f / 9.0f;

    Vector3 m_position = Vector3::Zero;
    Vector3 m_rightDir = Vector3::Right;
    Vector3 m_upDir = Vector3::Up;
    Vector3 m_forwardDir = Vector3::UnitZ;

    Matrix m_viewMatrix;
    Matrix m_projMatrix;
    Matrix m_viewProjMatrix;
    Matrix m_prevViewProjMatrix;

  public:
    BOOL m_usePerspectiveProjection = TRUE;
    BOOL m_isUpdated = FALSE;

  private:
    void UpdateProjMatrix();
    void UpdateViewMatrix();

  public:
    void Initialize(float verticalFovRadians, float aspectRatio, float nearZ, float farZ);

    void Update();

    void SetEyeAtUp(Vector3 eye, Vector3 at, Vector3 up);
    void SetLookDirection(Vector3 forward, Vector3 up);
    void SetPosition(Vector3 worldPos);

    //void SetPitch(float pitch);
    //void SetYaw(float yaw);
    void SetYawPitchRoll(float yaw, float pitch, float roll);

    void SetPerspective(float verticalFovRadians, float aspectRatio, float nearZ, float farZ);
    void SetFOV(float verticalFovRadians);
    void SetAspectRatio(float aspectRatio);
    void SetZRange(float nearZ, float farZ);

    void EnablePerspectiveProjection();
    void DisablePerspectiveProjection();

    const float GetNearClip() const { return m_nearZ; }
    const float GetFarClip() const { return m_farZ; }

    const Vector3 &GetRightDir() const { return m_rightDir; }
    const Vector3 &GetUpDir() const { return m_upDir; }
    const Vector3 &GetForwardDir() const { return m_forwardDir; }
    const Vector3 &GetPosition() const { return m_position; }

    const Matrix &GetViewMatrix() const { return m_viewMatrix; }
    const Matrix &GetProjMatrix() const { return m_projMatrix; }
    const Matrix &GetViewProjMatrix() const { return m_viewProjMatrix; }
};
