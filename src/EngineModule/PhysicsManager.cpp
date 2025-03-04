#include "pch.h"

#include "BroadPhase.h"
#include "GameObject.h"

#include "BoxCollider.h"
#include "RigidBody.h"
#include "SphereCollider.h"

#include "PhysicsManager.h"

BOOL PhysicsManager::Intersect(RigidBody *pA, RigidBody *pB, const float dt, Contact *pOutContact)
{
    if (!pA || !pB)
        return FALSE;

    pOutContact->pA = pA;
    pOutContact->pB = pB;

    Vector3 posA = pA->GetPosition();
    Vector3 posB = pB->GetPosition();

    Vector3 velA = pA->GetVelocity();
    Vector3 velB = pB->GetVelocity();

    if (pA->m_pCollider->GetType() == COLLIDER_TYPE_SPHERE && pB->m_pCollider->GetType() == COLLIDER_TYPE_SPHERE)
    {
        SphereCollider *pSphereA = (SphereCollider *)pA->m_pCollider;
        SphereCollider *pSphereB = (SphereCollider *)pB->m_pCollider;

        float   timeOfImpact;
        Vector3 contactPointAWorldSpace;
        Vector3 contactPointBWorldSpace;
        if (SphereSphereDynamic(pSphereA->GetRadius(), pSphereB->GetRadius(), posA, posB, velA, velB, dt,
                                &contactPointAWorldSpace, &contactPointBWorldSpace, &timeOfImpact))
        {
            pA->Update(timeOfImpact);
            pB->Update(timeOfImpact);

            pOutContact->contactPointAWorldSpace = contactPointAWorldSpace;
            pOutContact->contactPointBWorldSpace = contactPointBWorldSpace;

            // Convert world space contacts to local space
            pOutContact->contactPointALocalSpace = pA->WorldSpaceToLocalSpace(contactPointAWorldSpace);
            pOutContact->contactPointBLocalSpace = pB->WorldSpaceToLocalSpace(contactPointBWorldSpace);

            pOutContact->normal = pA->GetPosition() - pB->GetPosition();
            pOutContact->normal.Normalize();

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
}

BOOL PhysicsManager::Initialize() { return 0; }

BOOL PhysicsManager::CollisionTest(GameObject *pObj, const float dt)
{
    RigidBody *pCurComp = (RigidBody*)pObj->GetRigidBody();
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
        if (Intersect(pCurComp, pOtherComp, dt, &contact))
        {
            m_pContact[m_contactCount] = contact;
            m_contactCount++;
            return TRUE;
        }

        pCur = pCur->pNext;
    }
    return FALSE;
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
        qsort(m_pContact, m_contactCount, sizeof(Contact), CompareContacts);
    }

    float accumulatedTime = 0.0f;
    for (UINT i = 0; i < m_contactCount; i++)
    {
        Contact    &contact = m_pContact[i];
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
}
