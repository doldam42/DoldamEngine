#pragma once

#include "ControllerRegistry.h"

class RaytracingDemoController : public IController
{
  private:
    IGameObject *m_pGround = nullptr;
    IGameObject *m_pBox = nullptr;
    IGameObject *m_pGura = nullptr;

    void Cleanup();

  public:
    // Inherited via IController
    BOOL Start() override;
    void Update(float dt) override;

    RaytracingDemoController() = default;
    ~RaytracingDemoController();
};

