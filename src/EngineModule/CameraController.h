#pragma once

#include "../MathModule/MathHeaders.h"

class GameObject;

class CameraController
{
    Camera     *m_pCamera = nullptr;
    GameObject *m_pTarget = nullptr;

    float m_speed = 3.0f; // 움직이는 속도

    float m_prevCursorNDCX = 0.0f;
    float m_prevcursorNDCY = 0.0f;

    BoundingFrustum m_frustumVS;
    BoundingFrustum m_frustumWS;

  public:
    BOOL m_useFirstPersonView = TRUE;

  private:
    void UpdateKeyboard(const float dt);
    void UpdateMouse(float mouseNdcX, float mouseNdcY);

    void Cleanup();

  public:
    void Initialize(float verticalFovRadians, float aspectRatio, float nearZ, float farZ);

    const Vector3 &Eye() const { return m_pCamera->GetPosition(); }
    const Vector3 &LookAt() const { return m_pCamera->GetForwardDir(); }

    const Matrix &GetViewRow() const { return m_pCamera->GetViewMatrix(); }
    const Matrix &GetProjRow() const { return m_pCamera->GetProjMatrix(); }
    const Matrix &GetViewProjRow() const { return m_pCamera->GetViewProjMatrix(); }

    const BoundingFrustum &GetViewSpaceFrustum() const { return m_frustumVS; }
    const BoundingFrustum &GetWorldSpaceFrustum() const { return m_frustumWS; }

    void Update(const float dt);

    void MoveForward(float dt);
    void MoveRight(float dt);
    void MoveUp(float dt);

    void SetAspectRatio(float aspect);
    void SetFollowTarget(GameObject *pTarget);

    CameraController();
    ~CameraController();
};
