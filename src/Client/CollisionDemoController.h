#pragma once
#include "ControllerRegistry.h"

class CollisionDemoController : public IController // or AutoRegisterController<TimeController>
{
    static constexpr float SPEED = 10.0f;

    float speed = SPEED;
    float jumpSpeed = 0.0f;

    IGameObject *m_pPlayer = nullptr;

  public:
    // Inherited via IController
    BOOL Start() override;
    void Update(float dt) override;

    CollisionDemoController() {}
};
