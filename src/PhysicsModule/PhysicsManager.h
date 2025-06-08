#pragma once

#include "BroadPhase.h"
#include "Contact.h"

struct Collider;

constexpr UINT MAX_PAIR_PER_COLLIDER = 7;

struct ColliderData
{
    UINT PairCount = 0;
    UINT PairIndices[MAX_PAIR_PER_COLLIDER] = {0};
    UINT ContactIndices[MAX_PAIR_PER_COLLIDER] = {0};
};

class PhysicsManager : public IPhysicsManager
{
  public:
    static constexpr size_t MAX_BODY_COUNT = 1024;
    static constexpr size_t MAX_COLLISION_CANDIDATE_COUNT = 2048;
    static constexpr size_t MAX_COLLISION_COUNT = 512;

  private:
    BroadPhase *m_pBroadPhase = nullptr;

    Collider *m_pColliders[MAX_BODY_COUNT] = {nullptr};
    UINT      m_colliderCount = 0;

    CollisionPair m_collisionPairs[MAX_COLLISION_CANDIDATE_COUNT];
    Contact       m_contacts[MAX_COLLISION_COUNT];
    UINT          m_contactCount = 0;

    ColliderData m_colliderData[MAX_BODY_COUNT] = {};  // 콜라이더 별 충돌 데이터

    ULONG m_refCount = 1;

    BOOL Intersect(Collider *pA, Collider *pB, Contact* pOutContact);

  public:
    BOOL Initialize() override;

    ICollider *CreateSphereCollider(IGameObject* pObj, const float radius) override;
    ICollider *CreateBoxCollider(IGameObject *pObj, const Vector3 &halfExtents) override;
    ICollider *CreateEllipsoidCollider(IGameObject *pObj, const float majorRadius, const float minorRadius) override;
    void       DeleteCollider(ICollider *pDel) override;

    BOOL Raycast(const Ray &ray, Vector3 *pOutNormal, float *tHit, ICollider **pCollider) override;

    void BeginCollision(float dt) override;
    BOOL CollisionTestAll(float dt) override;
    void EndCollision() override;

    const ColliderData &GetColliderData(UINT colliderID) const { return m_colliderData[colliderID]; }
    ICollider *GetCollider(UINT colliderID) const { return m_pColliders[colliderID]; }
    const Contact &GetContact(UINT contactIdx) const { return m_contacts[contactIdx]; } 

    HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject) override;
    ULONG __stdcall AddRef(void) override;
    ULONG __stdcall Release(void) override;
};

extern PhysicsManager *g_pPhysics;