#pragma once

#include "../MathModule/MathHeaders.h"

class Client;
class CameraController : public IController
{
    IGameManager *m_pGame = nullptr;
    IGameObject  *m_pTarget = nullptr;

    float m_speed = 3.0f; // 움직이는 속도

    float m_prevCursorNDCX = 0.0f;
    float m_prevcursorNDCY = 0.0f;
    
  public:
    BOOL m_useFirstPersonView = TRUE;
    BOOL m_isFreezed = FALSE;

  private:
    void UpdateKeyboard(const float dt);
    void UpdateMouse(float mouseNdcX, float mouseNdcY);

    void Cleanup();

  public:
    void Initialize(Client *pClient);

    void Update(const float dt) override;

    void MoveForward(float dt);
    void MoveRight(float dt);
    void MoveUp(float dt);

    void SetFollowTarget(IGameObject *pTarget);

    CameraController();
    ~CameraController();

    // Inherited via IController
    BOOL Start() override;
    void Render() override;
};
