#include "pch.h"

#include "PhysicsManager.h"

#include "ColliderBase.h"

UINT Collider::GetCollidingColliders(ICollider **ppOutColliders, UINT maxColliders)
{
    if (!ppOutColliders || maxColliders == 0)
        return 0;

    const ColliderData &data = g_pPhysics->GetColliderData(ID);
    for (int i = 0; i < data.PairCount; i++)
    {
        UINT pairIndex = data.PairIndices[i];

        ICollider *other = g_pPhysics->GetCollider(pairIndex);
        ppOutColliders[i] = other;
    }

    return data.PairCount;
}
