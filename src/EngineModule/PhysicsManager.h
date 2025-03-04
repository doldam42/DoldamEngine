#pragma once

#include "Contact.h"

class GameObject;
class RigidBody;
class PhysicsManager
{
  private:
    Contact m_pContact[1024];
    UINT    m_contactCount = 0;

    RigidBody *m_pBodies[1024];
    UINT              m_bodyCount = 0;

    BOOL Intersect(RigidBody *pA, RigidBody *pB, const float dt, Contact *pOutContact);

  public:
    BOOL Initialize();

    BOOL CollisionTest(GameObject *pObj, const float dt);
    void ResolveContactsAll(float dt);
};
