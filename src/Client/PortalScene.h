#pragma once

#include "SceneRegistry.h"

class PortalScene : public IScene
{
    // Inherited via IScene
    void Load() override;
    void Update(float dt) override;
    void UnLoad() override;
};
