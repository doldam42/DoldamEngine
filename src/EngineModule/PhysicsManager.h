#pragma once

#include "Contact.h"
#include "BroadPhase.h"
#include "Maniford.h"

class World;
class GameObject;
class RigidBody;
class PhysicsManager
{
  public:
    static constexpr size_t MAX_COLLISION_COUNT = 256;
    static constexpr size_t MAX_BODY_COUNT = 1024;
    static constexpr size_t MAX_COLLISION_CANDIDATE_COUNT = MAX_BODY_COUNT * 3; 
  private:
    Contact m_contacts[MAX_COLLISION_COUNT] = {};
    UINT    m_contactCount = 0;

    RigidBody *m_pBodies[MAX_BODY_COUNT] = {nullptr};
    UINT       m_bodyCount = 0;

    CollisionPair m_collisionPairs[MAX_COLLISION_CANDIDATE_COUNT];

    BroadPhase *m_pBroadPhase = nullptr;

    std::map<ManifordKey, Maniford> m_manifords;

    BOOL ConservativeAdvance(RigidBody *pA, RigidBody *pB, float dt, Contact *pOutContact);

    BOOL Intersect(RigidBody *pA, RigidBody *pB, const float dt, Contact *pOutContact);
    BOOL Intersect(RigidBody *pA, RigidBody *pB, Contact *pOutContact);
    void Cleanup();

    void AddContact(const Contact &contact);
    void RemoveExpired();

  public:
    BOOL Initialize();

    RigidBody *CreateRigidBody(GameObject *pObj, ICollider *pCollider, float mass, float elasticity, float friction,
                               BOOL useGravity = TRUE, BOOL isKinematic = FALSE);
    void       DeleteRigidBody(RigidBody *pBody);

    void BeginCollision(float dt);

    void ApplyGravityImpulseAll(float dt);

    BOOL CollisionTest(GameObject *pObj, const float dt);
    BOOL CollisionTestAll(World* pWorld, const float dt);
    void ResolveContactsAll(float dt);

    bool IntersectRay(const Ray &ray, RayHit *pOutHit);

    PhysicsManager() = default;
    ~PhysicsManager();
};
