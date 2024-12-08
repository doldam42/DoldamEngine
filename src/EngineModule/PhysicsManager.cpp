#include "pch.h"

#include "GameObject.h"
#include "BroadPhase.h"
#include "Intersect.h"
#include "PhysicsComponent.h"
#include "PhysicsManager.h"

BOOL PhysicsManager::Initialize() { return 0; }

BOOL PhysicsManager::CollisionTest(GameObject *pObj, const float dt)
{
    SORT_LINK        *pCur = pObj->m_LinkInGame.pNext;
    PhysicsComponent *pCurComp = pObj->GetPhysicsComponent();

    m_pBodies[m_bodyCount] = pCurComp;
    m_bodyCount++;

    while (pCur)
    {
        GameObject       *pOther = (GameObject *)pCur->pItem;
        PhysicsComponent *pOtherComp = pOther->GetPhysicsComponent();

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
        Contact &contact = m_pContact[i];
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
