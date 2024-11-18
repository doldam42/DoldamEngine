#pragma once

#include "Contact.h"

class GameObject;
class PhysicsManager
{
  private:
    Contact m_pContact[1024];
    UINT    m_collisionCount = 0;

  public:
    BOOL Initialize();

    BOOL CollisionTest(GameObject *pObj);
    void ResolveContactsAll();
};
