#include "pch.h"

#include "GameObject.h"

#include "BoxCollider.h"
#include "RigidBody.h"
#include "SphereCollider.h"

#include "GJK.h"

#include "World.h"

#include "PhysicsManager.h"

BOOL PhysicsManager::ConservativeAdvance(RigidBody *pA, RigidBody *pB, float dt, Contact *pOutContact)
{ 
    Contact contact;
    contact.pA = pA;
    contact.pB = pB;

    float toi = 0.0f;

    int numIters = 0;

    // Advance the position of the bodies until they touch or there's not time left
    while (dt > 0.0f)
    {
        if (Intersect(pA, pB, &contact))
        {
            contact.timeOfImpact = toi;
            pA->Update(-toi);
            pB->Update(-toi);

            *pOutContact = contact;
            return TRUE;
        }

        ++numIters;
        if (numIters > 10)
            break;

        // Get the Vector from the closest point on A to the Closest point on B
        Vector3 ab = contact.contactPointBWorldSpace - contact.contactPointAWorldSpace;
        ab.Normalize();

        // Project the relative velocity onto the ray of shortest distance
        Vector3 relativeVelocity = pA->m_linearVelocity - pB->m_linearVelocity;
        float   orthoSpeed = relativeVelocity.Dot(ab);

        // Add to the orthoSpeed the maximum angular speed of the relative shape
        float angularSpeedA = pA->m_pCollider->FastestLinearSpeed(pA->m_angularVelocity, ab);
        float angularSpeedB = pB->m_pCollider->FastestLinearSpeed(pB->m_angularVelocity, -ab);

        orthoSpeed += angularSpeedA + angularSpeedB;

        if (orthoSpeed <= 0.0f)
        {
            break;
        }

        float timeToGo = contact.separationDistance / orthoSpeed;
        if (timeToGo > dt)
        {
            break;
        }

        dt -= timeToGo;
        toi += timeToGo;

        pA->Update(timeToGo);
        pB->Update(timeToGo);
    }

    pA->Update(-toi);
    pB->Update(-toi);

    *pOutContact = contact;
    return FALSE;
}

BOOL PhysicsManager::Intersect(RigidBody *pA, RigidBody *pB, const float dt, Contact *pOutContact)
{
    if (!pA || !pB)
        return FALSE;

    pOutContact->pA = pA;
    pOutContact->pB = pB;
    pOutContact->timeOfImpact = 0.0f;

    Vector3 posA = pA->GetPosition();
    Vector3 posB = pB->GetPosition();

    Vector3 velA = pA->GetVelocity();
    Vector3 velB = pB->GetVelocity();

    COLLIDER_TYPE typeA = pA->m_pCollider->GetType();
    COLLIDER_TYPE typeB = pB->m_pCollider->GetType();

    if (typeA == COLLIDER_TYPE_SPHERE && typeB == COLLIDER_TYPE_SPHERE)
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

            pOutContact->contactPointAWorldSpace = contactPointAWorldSpace;
            pOutContact->contactPointBWorldSpace = contactPointBWorldSpace;

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

BOOL PhysicsManager::Intersect(RigidBody *pA, RigidBody *pB, Contact *pOutContact)
{
    Contact contact;
    contact.pA = pA;
    contact.pB = pB;
    contact.timeOfImpact = 0.0f;

    Vector3 posA = pA->GetPosition();
    Vector3 posB = pB->GetPosition();

    if (pA->m_pCollider->GetType() == COLLIDER_TYPE_SPHERE && pB->m_pCollider->GetType() == COLLIDER_TYPE_SPHERE)
    {
        const SphereCollider *pSphereA = (const SphereCollider *)pA->m_pCollider;
        const SphereCollider *pSphereB = (const SphereCollider *)pB->m_pCollider;

        float radiusA = pSphereA->GetRadius();
        float radiusB = pSphereB->GetRadius();
        if (SphereSphereStatic(radiusA, radiusB, posA, posB, &contact.contactPointAWorldSpace,
                               &contact.contactPointBWorldSpace))
        {
            contact.normal = posA - posB;
            contact.normal.Normalize();

            // Convert world space contacts to local space
            contact.contactPointALocalSpace = pA->WorldSpaceToLocalSpace(contact.contactPointAWorldSpace);
            contact.contactPointBLocalSpace = pB->WorldSpaceToLocalSpace(contact.contactPointBWorldSpace);

            Vector3 ab = pB->GetPosition() - pA->GetPosition();
            float   r = ab.Length() - (pSphereA->GetRadius() + pSphereB->GetRadius());
            contact.separationDistance = r;

            *pOutContact = contact;
            return TRUE;
        }
    }
    else
    {
        Vector3 ptOnA, ptOnB;
        const float bias = 0.001f;
        if (GJK_DoesIntersect(pA, pB, bias, ptOnA, ptOnB))
        {
            Vector3 normal = ptOnB - ptOnA;
            normal.Normalize();

            ptOnA -= normal * bias;
            ptOnB += normal * bias;

            contact.normal = normal;

            contact.contactPointAWorldSpace = ptOnA;
            contact.contactPointBWorldSpace = ptOnB;
            
            // Convert world space contacts to local space
            contact.contactPointALocalSpace = pA->WorldSpaceToLocalSpace(contact.contactPointAWorldSpace);
            contact.contactPointBLocalSpace = pB->WorldSpaceToLocalSpace(contact.contactPointBWorldSpace);

            float   r = (ptOnA - ptOnB).Length();
            contact.separationDistance = r;

            *pOutContact = contact;
            return TRUE;
        }

        // there was no collision, but we still want the contact data, so get it
        GJK_ClosestPoints(pA, pB, ptOnA, ptOnB);
        contact.contactPointAWorldSpace = ptOnA;
        contact.contactPointBWorldSpace = ptOnB;

        // Convert world space contacts to local space
        contact.contactPointALocalSpace = pA->WorldSpaceToLocalSpace(contact.contactPointAWorldSpace);
        contact.contactPointBLocalSpace = pB->WorldSpaceToLocalSpace(contact.contactPointBWorldSpace);
        
        float r = (ptOnA - ptOnB).Length();
        contact.separationDistance = r;

        *pOutContact = contact;
    }
    return FALSE;
}

void PhysicsManager::Cleanup() {}

BOOL PhysicsManager::Initialize()
{
    m_collisionPairs.reserve(MAX_COLLISION_COUNT);
    return TRUE;
}

RigidBody *PhysicsManager::CreateRigidBody(GameObject *pObj, ICollider *pCollider, float mass, float elasticity,
                                           float friction, BOOL useGravity, BOOL isKinematic)
{
    RigidBody *pItem = new RigidBody;
    pItem->Initialize(pObj, pCollider, mass, elasticity, friction, useGravity, isKinematic);

    LinkToLinkedList(&m_pRigidBodyLinkHead, &m_pRigidBodyLinkTail, &pItem->m_linkInPhysics);

    return pItem;
}

void PhysicsManager::DeleteRigidBody(RigidBody *pBody)
{
    UnLinkFromLinkedList(&m_pRigidBodyLinkHead, &m_pRigidBodyLinkTail, &pBody->m_linkInPhysics);
    delete pBody;
}

void PhysicsManager::ApplyGravityImpulseAll(float dt)
{
    SORT_LINK *pCur = m_pRigidBodyLinkHead;
    while (pCur)
    {
        RigidBody *pBody = (RigidBody *)pCur->pItem;

        pBody->ApplyGravityImpulse(dt);

        pCur = pCur->pNext;
    }
}

BOOL PhysicsManager::CollisionTest(GameObject *pObj, const float dt)
{
    RigidBody *pCurComp = (RigidBody *)pObj->GetRigidBody();
    if (!pCurComp)
        return FALSE;

    m_pBodies[m_bodyCount] = pCurComp;
    m_bodyCount++;

    SORT_LINK *pCur = pObj->m_LinkInGame.pNext;
    while (pCur)
    {
        GameObject *pOther = (GameObject *)pCur->pItem;
        RigidBody  *pOtherComp = (RigidBody *)pOther->GetRigidBody();

        Contact contact;
        if (!pCurComp->m_isKinematic && !pOtherComp->m_isKinematic && Intersect(pCurComp, pOtherComp, dt, &contact))
        {
            m_contacts[m_contactCount] = contact;
            m_contactCount++;
            return TRUE;
        }

        pCur = pCur->pNext;
    }
    return FALSE;
}

BOOL PhysicsManager::CollisionTestAll(World *pWorld, const float dt)
{
    SORT_LINK *pCur = m_pRigidBodyLinkHead;
    while (pCur)
    {
        RigidBody *pBody = (RigidBody *)pCur->pItem;

        m_pBodies[m_bodyCount] = pBody;
        m_bodyCount++;

        pCur = pCur->pNext;
    }

    BroadPhase(m_pBodies, m_bodyCount, m_collisionPairs, dt);

    for (const CollisionPair &pair : m_collisionPairs)
    {
        RigidBody *pA = m_pBodies[pair.a];
        RigidBody *pB = m_pBodies[pair.b];

        if (pA->m_invMass == 0 && pB->m_invMass == 0)
            continue;
        if (pA->m_isKinematic || pB->m_isKinematic)
            continue;

        Contact contact;
        if (Intersect(pA, pB, dt, &contact))
        {
            m_contacts[m_contactCount] = contact;
            m_contactCount++;
        }
    }

    return TRUE;
}

int CompareContacts(const void *p1, const void *p2)
{
    Contact a = *(Contact *)p1;
    Contact b = *(Contact *)p2;

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

void PhysicsManager::ResolveContactsAll(float dt)
{
    if (m_contactCount > 1)
    {
        qsort(m_contacts, m_contactCount, sizeof(Contact), CompareContacts);
    }

    float accumulatedTime = 0.0f;
    for (UINT i = 0; i < m_contactCount; i++)
    {
        Contact    &contact = m_contacts[i];
        const float dt_sec = contact.timeOfImpact - accumulatedTime;

        // position Update
        for (int j = 0; j < m_bodyCount; j++)
        {
            m_pBodies[j]->Update(dt_sec);
        }
        ResolveContact(contact);
        accumulatedTime += dt_sec;
    }

    // Update the positions for the rest of this frame's time
    const float timeRemaining = dt - accumulatedTime;
    if (timeRemaining > 0.0f)
    {
        for (int i = 0; i < m_bodyCount; i++)
        {
            m_pBodies[i]->Update(timeRemaining);
        }
    }

    m_contactCount = 0;
    m_bodyCount = 0;
    m_collisionPairs.clear();
}

PhysicsManager::~PhysicsManager() {}
