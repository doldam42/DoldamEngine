#include "pch.h"

#include "GameObject.h"

#include "PhysicsManager.h"

void PhysicsManager::CollisionAndResolve(SORT_LINK *pGameObjLink, const UINT objectCount)
{
    SORT_LINK *pCur = pGameObjLink;
    while (pCur)
    {
        GameObject *pCurObj = (GameObject *)pCur->pItem;

        Box    box;
        ShapeSphere sphere;

        Shape *pShape = nullptr;

        if (pCurObj->m_collisionShapeType == SHAPE_TYPE_BOX)
        {
            pCurObj->GetBoxInWorld(&box);
            pShape = &box;
        }
        else if (pCurObj->m_collisionShapeType == SHAPE_TYPE_SPHERE)
        {
        }

        SORT_LINK *pOther = pCur->pNext;
        while (pOther)
        {
            GameObject *pOtherObj = (GameObject *)pOther->pItem;

            if (pCurObj->m_invMass != 0 || pOtherObj->m_invMass != 0)
            {
                Contact contact;
                if (pCurObj->Intersect(pOtherObj, &contact))
                {
                    GameObject::ResolveContact(pCurObj, pOtherObj, contact);
                }
            }

            pOther = pOther->pNext;
        }

        pCur = pCur->pNext;
    }
}
