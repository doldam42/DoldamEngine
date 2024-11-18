#include "pch.h"

#include "GameObject.h"
#include "Intersect.h"
#include "PhysicsComponent.h"
#include "PhysicsManager.h"

BOOL PhysicsManager::Initialize() { return 0; }

BOOL PhysicsManager::CollisionTest(GameObject *pObj)
{
    SORT_LINK        *pCur = pObj->m_LinkInGame.pNext;
    PhysicsComponent *pCurComp = pObj->GetPhysicsComponent();

    while (pCur)
    {
        GameObject       *pOther = (GameObject *)pCur->pItem;
        PhysicsComponent *pOtherComp = pOther->GetPhysicsComponent();

        Contact contact;
        if (Intersect(pCurComp, pOtherComp, &contact))
        {
            m_pContact[m_collisionCount] = contact;
            m_collisionCount++;
            return TRUE;
        }

        pCur = pCur->pNext;
    }
    return FALSE;
}

void PhysicsManager::ResolveContactsAll()
{
    for (UINT i = 0; i < m_collisionCount; i++)
    {
        ResolveContact(m_pContact[i]);
    }
    m_collisionCount = 0;
}
