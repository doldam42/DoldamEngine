#include "pch.h"

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

BOOL PhysicsManager::Initialize() { return TRUE; }

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
}

BOOL PhysicsManager::CollisionTestAll(float dt) { return 0; }

void PhysicsManager::EndCollision() {}
