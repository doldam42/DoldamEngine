#pragma once

#include "Contact.h"

class GameObject;
class PhysicsComponent;
class PhysicsManager
{
  private:
    Contact m_pContact[1024];
    UINT    m_contactCount = 0;

    PhysicsComponent *m_pBodies[1024];
    UINT              m_bodyCount = 0;

    BOOL Intersect(PhysicsComponent *pA, PhysicsComponent *pB, const float dt, Contact *pOutContact);

  public:
    BOOL Initialize();

    BOOL CollisionTest(GameObject *pObj, const float dt);
    void ResolveContactsAll(float dt);
};
