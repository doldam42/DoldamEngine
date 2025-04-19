#pragma once
#include "CharacterController.h"
class PlayerController : public IController
{
    static constexpr float SPEED = 8.0f;
    static constexpr float JUMP_SPEED = 10.0f;

    CharacterController *m_pController = nullptr;

    IGameObject *m_pPortal = nullptr;

  public:
    BOOL Start() override;
    void Update(float dt) override;
};
