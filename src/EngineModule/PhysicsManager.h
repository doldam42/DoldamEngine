#pragma once

#include "Contact.h"
#include "BroadPhase.h"

class World;
class GameObject;
class RigidBody;
class PhysicsManager
{
  public:
    static constexpr size_t MAX_COLLISION_COUNT = 256;
    static constexpr size_t MAX_BODY_COUNT = 1024;
  private:
    Contact m_contacts[MAX_COLLISION_COUNT] = {};
    UINT    m_contactCount = 0;

    RigidBody *m_pBodies[MAX_BODY_COUNT] = {nullptr};
    UINT       m_bodyCount = 0;

    std::vector<CollisionPair> m_collisionPairs;

    SORT_LINK *m_pRigidBodyLinkHead = nullptr;
    SORT_LINK *m_pRigidBodyLinkTail = nullptr;

    BOOL Intersect(RigidBody *pA, RigidBody *pB, const float dt, Contact *pOutContact);

    void Cleanup();

  public:
    BOOL Initialize();

    RigidBody *CreateRigidBody(GameObject *pObj, ICollider *pCollider, float mass, float elasticity, float friction,
                               BOOL useGravity = TRUE, BOOL isKinematic = FALSE);
    void       DeleteRigidBody(RigidBody *pBody);

    void ApplyGravityImpulseAll(float dt);

    BOOL CollisionTest(GameObject *pObj, const float dt);
    BOOL CollisionTestAll(World* pWorld, const float dt);
    void ResolveContactsAll(float dt);

    PhysicsManager() = default;
    ~PhysicsManager();
};
