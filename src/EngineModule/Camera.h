#pragma once

#include "../MathModule/MathHeaders.h"

class GameObject;

class Camera
{
    void UpdateViewDir();
    void UpdateKeyboard(const float dt);
    void UpdateMouse(float mouseNdcX, float mouseNdcY);
    
    void UpdateProjMatrix();
    void UpdateViewMatrix();

    void UpdateFrustum();

  public:
    Camera()
    {
        UpdateViewDir();
        UpdateProjMatrix();
    }

    const Matrix &GetViewRow() const { return m_viewMatrix; }
    const Matrix &GetProjRow() const { return m_projMatrix; }
    const Matrix &GetViewProjRow() const { return m_viewProjMatrix; }

    const BoundingFrustum &GetViewSpaceFrustum() const { return m_frustumVS; }
    const BoundingFrustum &GetWorldSpaceFrustum() const { return m_frustumWS; }

    void Update(const float dt);

    void MoveForward(float dt);
    void MoveRight(float dt);
    void MoveUp(float dt);
    void SetAspectRatio(float aspect);
    void SetFollowTarget(GameObject *pTarget);
    void SetViewMatrix(const Matrix &viewMat) { m_viewMatrix = viewMat; }
    void SetProjMatrix(const Matrix &projMat) { m_projMatrix = projMat; }
    void SetViewProjMatrix(const Matrix &viewProjMat) { m_viewProjMatrix = viewProjMat; }

    const Vector3 &Eye() const { return m_viewDir; }
    const Vector3 &At() const { return m_position; }
    const Vector3 &Up() const { return m_upDir; }

  public:
    bool m_useFirstPersonView = false;
    bool m_isUpdated = false;

  private:
    GameObject *m_pTarget = nullptr;

    Vector3 m_position = Vector3(0.644748f, 0.151557f, 0.898394f);
    Vector3 m_viewDir = Vector3::UnitZ;
    Vector3 m_upDir = Vector3::UnitY; // 이번 예제에서는 고정
    Vector3 m_rightDir = Vector3::UnitX;

    Matrix m_viewMatrix;
    Matrix m_projMatrix;
    Matrix m_viewProjMatrix;
    Matrix m_prevViewProjMatrix;

    // roll, pitch, yaw
    // https://en.wikipedia.org/wiki/Aircraft_principal_axes
    float m_yaw = -0.618501f, m_pitch = -0.0785397f;

    float m_speed = 3.0f; // 움직이는 속도

    // 프로젝션 옵션도 카메라 클래스로 이동
    float m_projFovAngleY = 90.0f;
    float m_nearZ = 0.01f;
    float m_farZ = 100.0f;
    float m_aspect = 16.0f / 9.0f;
    bool  m_usePerspectiveProjection = true;

    float m_prevCursorNDCX = 0.0f;
    float m_prevcursorNDCY = 0.0f;

    BoundingFrustum m_frustumVS;
    BoundingFrustum m_frustumWS;
};
