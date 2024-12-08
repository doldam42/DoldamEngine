#pragma once

#include "Contact.h"

class GameObject;
class PhysicsManager
{
  private:
    Contact m_pContact[1024];
    UINT    m_contactCount = 0;

    PhysicsComponent *m_pBodies[1024];
    UINT              m_bodyCount = 0;

  public:
    BOOL Initialize();

    BOOL CollisionTest(GameObject *pObj, const float dt);
    void ResolveContactsAll(float dt);
};
