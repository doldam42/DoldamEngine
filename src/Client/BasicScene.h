#pragma once

#include "SceneRegistry.h"

class BasicScene : public IScene
{
    // crossHair
    int          m_crossHairPosX = 0;
    int          m_crossHairPosY = 0;
    int          m_crossHairImageSize = 256;
    float        m_crossHairScale = 0.25f;
    IGameSprite *m_pCrossHairSprite = nullptr;

  public:
    void Load() override;
    void Update(float dt) override;
    void UnLoad() override;

    BasicScene() = default;
    ~BasicScene() {}
};
