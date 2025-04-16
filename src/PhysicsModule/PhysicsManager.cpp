#include "pch.h"

#include "BroadPhase.h"
#include "ColliderBase.h"
#include "SphereCollider.h"

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

    if (typeA == COLLIDER_TYPE_SPHERE && typeB == COLLIDER_TYPE_SPHERE)
    {
        SphereCollider *pSphereA = (const SphereCollider *)pA;
        SphereCollider *pSphereB = (const SphereCollider *)pB;

        Vector3 contactPointA;
        Vector3 contactPointB;
        if (SphereSphereStatic(pSphereA->Radius, pSphereB->Radius, posA, posB, &contactPointA, &contactPointB))
        {
            pSphereA->IsCollide = TRUE;
            pSphereB->IsCollide = TRUE;
        }
    }
}

BOOL PhysicsManager::Initialize()
{
    m_pBroadPhase = new BroadPhase;
    m_pBroadPhase->Initialize(MAX_BODY_COUNT, Vector3::One);

    return TRUE;
}

ICollider *PhysicsManager::CreateSphereCollider(const float radius)
{
    SphereCollider *pNew = new SphereCollider(radius);
    m_pColliders[m_colliderCount] = pNew;
    pNew->ID = m_colliderCount;
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

void PhysicsManager::BeginCollision(float dt)
{
    for (int i = 0; i < m_colliderCount; i++)
    {
        Collider *pCollider = m_pColliders[i];
        pCollider->IsPrevCollide = pCollider->IsCollide;
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

        if (pA->m_invMass == 0 && pB->m_invMass == 0)
        {
            __debugbreak();
            continue;
        }
        if (pA->m_isKinematic || pB->m_isKinematic)
            continue;

        Contact contact;
        if (Intersect(pA, pB, dt, &contact))
        {
            if (contact.timeOfImpact == 0.0f)
            {
                // Static contact
                AddContact(contact);
            }
            else
            {
                m_contacts[m_contactCount] = contact;
                m_contactCount++;
            }
        }
    }
}

void PhysicsManager::EndCollision() {}
