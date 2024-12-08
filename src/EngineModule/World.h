#pragma once

#include "PhysicsComponent.h"

class World
{
  private:
    KDTree* m_pTree = nullptr;

  private:
    void Cleanup();
  public:
    void Initialize(GameObject **pObjectArray, int objectCount);
};
