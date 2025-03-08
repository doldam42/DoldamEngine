#pragma once

#include "../MathModule/MathHeaders.h"

class GameEditor;
class CameraController : public IController
{
    IGameManager *m_pGame = nullptr;
    IGameObject *m_pTarget = nullptr;

    float m_speed = 3.0f; // �����̴� �ӵ�

    float m_prevCursorNDCX = 0.0f;
    float m_prevcursorNDCY = 0.0f;

  public:
    BOOL m_useFirstPersonView = TRUE;

  private:
    void UpdateKeyboard(const float dt);
    void UpdateMouse(float mouseNdcX, float mouseNdcY);

    void Cleanup();

  public:
    void Initialize(GameEditor *pEditor);

    void Update(const float dt) override;

    void MoveForward(float dt);
    void MoveRight(float dt);
    void MoveUp(float dt);

    void SetFollowTarget(IGameObject *pTarget);

    CameraController();
    ~CameraController();

    // Inherited via IController
    BOOL Start() override;
};
