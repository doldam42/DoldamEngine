#pragma once
#include "ControllerRegistry.h"

enum SHOT_TYPE
{
    SHOT_TYPE_PISTOL = 0,
    SHOT_TYPE_ARTILLERY,
    SHOT_TYPE_FIREBALL,
    SHOT_TYPE_LASER,
};

struct Shot
{
    IGameObject *pObj;
    Particle     particle;
    SHOT_TYPE    shotType;
    float        startTime;
};
class BallisticDemo : public IController
{
    SHOT_TYPE m_shotType = SHOT_TYPE_PISTOL;

    std::vector<Shot> m_shots;
  public:
    // Inherited via IController
    BOOL Start() override;
    void Update(float dt) override;
};
