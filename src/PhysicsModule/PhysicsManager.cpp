#include "pch.h"

#include "BoxCollider.h"
#include "RigidBody.h"
#include "SphereCollider.h"

#include "GJK.h"

#include "PhysicsManager.h"

BOOL PhysicsManager::Intersect(RigidBody *pA, RigidBody *pB, const float dt, CollisionData *pOutCollision)
{
    if (!pA || !pB)
        return FALSE;

    pOutCollision->pA = pA;
    pOutCollision->pB = pB;
    pOutCollision->timeOfImpact = 0.0f;

    Vector3 posA = pA->GetPosition();
    Vector3 posB = pB->GetPosition();

    Vector3 velA = pA->GetVelocity();
    Vector3 velB = pB->GetVelocity();

    DP_COLLIDER_TYPE typeA = pA->m_pCollider->GetType();
    DP_COLLIDER_TYPE typeB = pB->m_pCollider->GetType();

    if (typeA == DP_COLLIDER_TYPE_SPHERE && typeB == DP_COLLIDER_TYPE_SPHERE)
    {
        SphereCollider *pSphereA = (SphereCollider *)pA->m_pCollider;
        SphereCollider *pSphereB = (SphereCollider *)pB->m_pCollider;

        float   timeOfImpact;
        Vector3 normal;
        Vector3 contactPointAWorldSpace;
        Vector3 contactPointBWorldSpace;
        if (SphereSphereDynamic(pSphereA->GetRadius(), pSphereB->GetRadius(), posA, posB, velA, velB, dt,
                                &contactPointAWorldSpace, &contactPointBWorldSpace, &normal, &timeOfImpact))
        {
            pA->Update(timeOfImpact);
            pB->Update(timeOfImpact);

            pOutContact->contacts[0].contactPointAWorldSpace = contactPointAWorldSpace;
            pOutContact->contacts[0].contactPointBWorldSpace = contactPointBWorldSpace;

            // Convert world space contacts to local space
            pOutContact->contactPointALocalSpace = pA->WorldSpaceToLocalSpace(contactPointAWorldSpace);
            pOutContact->contactPointBLocalSpace = pB->WorldSpaceToLocalSpace(contactPointBWorldSpace);

            pOutContact->normal = normal;
            pOutContact->timeOfImpact = timeOfImpact;

            // Unwind time step
            pA->Update(-timeOfImpact);
            pB->Update(-timeOfImpact);

            // Calculate the separation distance
            Vector3 ab = pB->GetPosition() - pA->GetPosition();
            float   r = ab.Length() - (pSphereA->GetRadius() + pSphereB->GetRadius());

            pOutContact->separationDistance = r;

            return TRUE;
        }
        return FALSE;
    }
    else
    {
        return ConservativeAdvance(pA, pB, dt, pOutContact);
    }

    return FALSE;
}

void PhysicsManager::Cleanup() {}

BOOL PhysicsManager::Initialize()
{
    m_pBroadPhase = new BroadPhase;
    m_pBroadPhase->Initialize(MAX_BODY_COUNT, Vector3::One);

    m_pTree = new BVH(MAX_BODY_COUNT);

    return TRUE;
}

RigidBody *PhysicsManager::CreateRigidBody(IGameObject *pObj, ICollider *pCollider, float mass, float elasticity,
                                           float friction, BOOL useGravity)
{
    RigidBody *pItem = new RigidBody;
    pItem->Initialize(pObj, pCollider, mass, elasticity, friction, useGravity);

    if (pItem->IsFixed())
    {
        m_pTree->InsertObject(pItem->GetBounds(), pItem);
    }

    pItem->id = m_bodyCount;
    m_pBodies[m_bodyCount] = pItem;
    m_bodyCount++;

    return pItem;
}

void PhysicsManager::DeleteRigidBody(RigidBody *pBody)
{
    UINT idx = pBody->id;

    RigidBody *pBack = m_pBodies[m_bodyCount - 1];

    m_pBodies[idx] = pBack;
    pBack->id = idx;
    m_bodyCount--;

    delete pBody;
}

void PhysicsManager::BeginCollision(float dt)
{
    ApplyGravityImpulseAll(dt);
    m_pBroadPhase->Build(m_pBodies, m_bodyCount, dt);
}

void PhysicsManager::EndCollision() { m_collisionCount = 0; }

void PhysicsManager::ApplyGravityImpulseAll(float dt)
{
    for (UINT i = 0; i < m_bodyCount; i++)
    {
        RigidBody *pBody = m_pBodies[i];
        pBody->ApplyGravityImpulse(dt);
    }
}

BOOL PhysicsManager::CollisionTestAll(const float dt)
{
    ZeroMemory(m_collisionPairs, sizeof(CollisionPair) * MAX_COLLISION_CANDIDATE_COUNT);
    UINT numCandidate = m_pBroadPhase->QueryCollisionPairs(m_collisionPairs, MAX_COLLISION_CANDIDATE_COUNT);

    for (int i = 0; i < m_bodyCount; i++)
    {
        const RigidBody *pBody = m_pBodies[i];
        if (pBody->IsFixed())
            continue;

        const Bounds &b = m_pBodies[i]->GetBounds();

        RigidBody *pOther = nullptr;
        if (m_pTree->Intersect(b, reinterpret_cast<void **>(&pOther)))
        {
            m_collisionPairs[numCandidate].a = pBody->id;
            m_collisionPairs[numCandidate].b = pOther->id;
            numCandidate++;
        }
    }

    for (UINT i = 0; i < numCandidate; i++)
    {
        const CollisionPair &pair = m_collisionPairs[i];

        RigidBody *pA = m_pBodies[pair.a];
        RigidBody *pB = m_pBodies[pair.b];

        if (pA->m_invMass == 0 && pB->m_invMass == 0)
        {
            __debugbreak();
            continue;
        }

        CollisionData collision;
        if (Intersect(pA, pB, dt, &collision))
        {
            m_collisions[m_collisionCount] = collision;
            m_collisionCount++;
        }
    }

    return TRUE;
}

int CompareCollision(const void *p1, const void *p2)
{
    CollisionData a = *(CollisionData *)p1;
    CollisionData b = *(CollisionData *)p2;

    if (a.timeOfImpact < b.timeOfImpact)
    {
        return -1;
    }

    if (a.timeOfImpact == b.timeOfImpact)
    {
        return 0;
    }

    return 1;
}
//
// void PhysicsManager::ResolveContactsAll(float dt)
//{
//    if (m_contactCount > 1)
//    {
//        qsort(m_contacts, m_contactCount, sizeof(Contact), CompareContacts);
//    }
//
//    for (auto &maniPair : m_manifords)
//    {
//        Maniford &manifold = maniPair.second;
//        for (int i = 0; i < manifold.m_numContacts; i++)
//        {
//            Contact &contact = manifold.m_contacts[i];
//            ResolveContact(contact);
//        }
//    }
//
//    float accumulatedTime = 0.0f;
//    for (UINT i = 0; i < m_contactCount; i++)
//    {
//        Contact    &contact = m_contacts[i];
//        const float dt_sec = contact.timeOfImpact - accumulatedTime;
//
//        // position Update
//        for (UINT j = 0; j < m_bodyCount; j++)
//        {
//            m_pBodies[j]->Update(dt_sec);
//        }
//        ResolveContact(contact);
//        accumulatedTime += dt_sec;
//    }
//
//    // Update the positions for the rest of this frame's time
//    const float timeRemaining = dt - accumulatedTime;
//    if (timeRemaining > 0.0f)
//    {
//        for (UINT i = 0; i < m_bodyCount; i++)
//        {
//            m_pBodies[i]->Update(timeRemaining);
//        }
//    }
//}

BOOL PhysicsManager::Raycast(const Ray &ray, float *tHit, IGameObject *pHitted) { return FALSE; }

void PhysicsManager::BuildScene() { m_pTree->Build(); }

PhysicsManager::~PhysicsManager() {}
