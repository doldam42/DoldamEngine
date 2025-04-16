#pragma once

#include "BroadPhase.h"

struct Collider;
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

    ULONG m_refCount = 1;

    BOOL Intersect(Collider *pA, Collider *pB);

  public:
    BOOL Initialize() override;

    ICollider *CreateSphereCollider(const float radius) override;
    void       DeleteCollider(ICollider *pDel) override;

    void BeginCollision(float dt) override;
    BOOL CollisionTestAll(float dt) override;
    void EndCollision() override;

    HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject) override;
    ULONG __stdcall AddRef(void) override;
    ULONG __stdcall Release(void) override;
};