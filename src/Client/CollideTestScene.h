#pragma once

#include "SceneRegistry.h"

class CollideTestScene : public IScene
{
    IGameObject *m_pSpheres[25] = {nullptr};

  public:
    void Load() override;
    void Update(float dt) override;
    void UnLoad() override;
};
