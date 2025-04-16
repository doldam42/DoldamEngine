#include "pch.h"

#include "BroadPhase.h"
#include "ColliderBase.h"
#include "SphereCollider.h"
#include "BoxCollider.h"

#include "PhysicsManager.h"

HRESULT __stdcall PhysicsManager::QueryInterface(REFIID riid, void **ppvObject) { return E_NOTIMPL; }

ULONG __stdcall PhysicsManager::AddRef(void)
{
    m_refCount++;
    return m_refCount;
}

ULONG __stdcall PhysicsManager::Release(void)
{
    ULONG ref_count = --m_refCount;
    if (!m_refCount)
        delete this;

    return ref_count;
}

BOOL PhysicsManager::Intersect(Collider *pA, Collider *pB) 
{ 
    COLLIDER_TYPE typeA = pA->GetType(); 
    COLLIDER_TYPE typeB = pB->GetType();

    Vector3 posA = pA->Position;
    Vector3 posB = pB->Position;

    if (typeA == COLLIDER_TYPE_SPHERE)
    {
        if (typeB == COLLIDER_TYPE_SPHERE)
        {
            const SphereCollider *pSphereB = (const SphereCollider *)pB;
            const SphereCollider *pSphereA = (const SphereCollider *)pA;

            Vector3 contactPointA;
            Vector3 contactPointB;
            if (SphereSphereStatic(pSphereA->Radius, pSphereB->Radius, posA, posB, &contactPointA, &contactPointB))
            {
                return TRUE;
            }
        }
        else if (typeB == COLLIDER_TYPE_BOX)
        {
            const SphereCollider *pSphere = (const SphereCollider *)pA;
            const BoxCollider    *pBox = (const BoxCollider *)pB;
            if (SphereBoxStatic(pSphere->Radius, posA, pBox->HalfExtent, pBox->Rotation, posB))
            {
                return TRUE;
            }
        }
    }
    else if (typeA == COLLIDER_TYPE_BOX)
    {
        if (typeB == COLLIDER_TYPE_SPHERE)
        {
            const BoxCollider    *pBox = (const BoxCollider *)pA;
            const SphereCollider *pSphere = (const SphereCollider *)pB;
            if (SphereBoxStatic(pSphere->Radius, posA, pBox->HalfExtent, pBox->Rotation, posB))
            {
                return TRUE;
            }
        }
        else if (typeB == COLLIDER_TYPE_BOX)
        {
            const BoxCollider *pBoxA = (const BoxCollider *)pA;
            const BoxCollider *pBoxB = (const BoxCollider *)pB;

            if (BoxBoxStatic(pBoxA->HalfExtent, pBoxB->HalfExtent, pBoxA->Rotation, pBoxB->Rotation, pBoxA->Position,
                             pBoxB->Position))
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

BOOL PhysicsManager::Initialize()
{
    m_pBroadPhase = new BroadPhase;
    m_pBroadPhase->Initialize(MAX_BODY_COUNT, Vector3::One);

    return TRUE;
}

ICollider *PhysicsManager::CreateSphereCollider(IGameObject *pObj, const float radius)
{
    SphereCollider *pNew = new SphereCollider(radius);
    m_pColliders[m_colliderCount] = pNew;
    pNew->ID = m_colliderCount;
    pNew->pObj = pObj;
    m_colliderCount++;
    return pNew;
}

ICollider *PhysicsManager::CreateBoxCollider(IGameObject *pObj, const Vector3 &halfExtents)
{
    BoxCollider *pNew = new BoxCollider(halfExtents);
    m_pColliders[m_colliderCount] = pNew;
    pNew->ID = m_colliderCount;
    pNew->pObj = pObj;
    m_colliderCount++;
    return pNew;
}

void PhysicsManager::DeleteCollider(ICollider *pDel)
{
    Collider *pCol = (Collider *)pDel;

    UINT idx = pCol->ID;
    m_pColliders[idx] = m_pColliders[m_colliderCount - 1];
    m_colliderCount--;
    m_pColliders[idx]->ID = idx;
    delete pDel;
}

BOOL PhysicsManager::Raycast(const Ray &ray, float *tHit, ICollider **pCollider) { return 0; }

void PhysicsManager::BeginCollision(float dt)
{
    for (int i = 0; i < m_colliderCount; i++)
    {
        Collider *pCollider = m_pColliders[i];
        pCollider->IsPrevCollide = pCollider->IsCollide;
        pCollider->IsCollide = FALSE;
    }

    m_pBroadPhase->Build(m_pColliders, m_colliderCount, dt);
}

BOOL PhysicsManager::CollisionTestAll(float dt)
{
    ZeroMemory(m_collisionPairs, sizeof(CollisionPair) * MAX_COLLISION_CANDIDATE_COUNT);
    UINT numCandidate = m_pBroadPhase->QueryCollisionPairs(m_collisionPairs, MAX_COLLISION_CANDIDATE_COUNT);
    for (UINT i = 0; i < numCandidate; i++)
    {
        const CollisionPair &pair = m_collisionPairs[i];

        Collider *pA = m_pColliders[pair.a];
        Collider *pB = m_pColliders[pair.b];

        if (pA->IsActive && pB->IsActive && Intersect(pA, pB))
        {
            pA->IsCollide = TRUE;
            pB->IsCollide = TRUE;
        }
    }

    return TRUE;
}

void PhysicsManager::EndCollision() {}
