#pragma once

#include "Contact.h"
#include "BroadPhase.h"

struct CollisionData
{
    // Á¢ÃË ¹è¿­
    Contact contacts[4];
    // Á¢ÃË ¹è¿­¿¡ ÀÖ´Â Ã¸ÃË °³¼ö
    UINT     contactsLeft;

    float timeOfImpact;

    IRigidBody *pA;
    IRigidBody *pB;
};

class PhysicsManager : public IPhysicsManager
{
  public:
    static constexpr size_t MAX_COLLISION_COUNT = 256;
    static constexpr size_t MAX_BODY_COUNT = 1024;
    static constexpr size_t MAX_COLLISION_CANDIDATE_COUNT = MAX_BODY_COUNT * 3; 
  private:
    BVH *m_pTree = nullptr;

    CollisionPair m_collisionPairs[MAX_COLLISION_CANDIDATE_COUNT];

    CollisionData m_collisions[MAX_COLLISION_COUNT] = {};
    UINT    m_collisionCount = 0;

    RigidBody *m_pBodies[MAX_BODY_COUNT] = {nullptr};
    UINT       m_bodyCount = 0;

    BroadPhase   *m_pBroadPhase = nullptr;

    BOOL Intersect(RigidBody *pA, RigidBody *pB, const float dt, CollisionData *pOutContact);

    void Cleanup();

  public:
    BOOL Initialize();

    RigidBody *CreateRigidBody(IGameObject *pObj, ICollider *pCollider, float mass, float elasticity, float friction,
                               BOOL useGravity = TRUE) override;
    void       DeleteRigidBody(RigidBody *pBody);

    void BeginCollision(float dt) override;
    void EndCollision() override;

    void ApplyGravityImpulseAll(float dt);

    BOOL CollisionTestAll(const float dt);

    BOOL Raycast(const Ray &ray, float *tHit, IGameObject *pHitted) override;

    void BuildScene() override;

    PhysicsManager() = default;
    ~PhysicsManager();
};
