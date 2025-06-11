#include "pch.h"

#include "PhysicsManager.h"

#include "Collider.h"

UINT Collider::GetCollidingColliders(ICollider **ppOutColliders, UINT maxColliders)
{
    const ColliderData &data = g_pPhysics->GetColliderData(ID);

    DASSERT(ppOutColliders);
    DASSERT(maxColliders > 0);
    DASSERT(maxColliders > data.PairCount);

    for (int i = 0; i < data.PairCount; i++)
    {
        UINT pairIndex = data.PairIndices[i];

        ICollider *other = g_pPhysics->GetCollider(pairIndex);
        ppOutColliders[i] = other;
    }

    return data.PairCount;
}

UINT Collider::QueryCollisionData(CollisionData **ppOutData, UINT maxCollision)
{
    const ColliderData &data = g_pPhysics->GetColliderData(ID);

    DASSERT(ppOutData);
    DASSERT(maxCollision > 0);
    DASSERT(maxCollision > data.PairCount);

    for (int i = 0; i < data.PairCount; i++)
    {
        UINT pairIndex = data.PairIndices[i];
        UINT contactIndex = data.ContactIndices[i];

        ICollider *other = g_pPhysics->GetCollider(pairIndex);
        const Contact& contact = g_pPhysics->GetContact(contactIndex);

        ppOutData[i]->pOther = other;
        ppOutData[i]->point = (contact.pA == this) ? contact.contactPointBWorldSpace : contact.contactPointAWorldSpace;
        ppOutData[i]->normal = contact.normal;
    }

    return data.PairCount;
}
