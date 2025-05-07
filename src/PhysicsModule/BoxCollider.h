#pragma once

#include "ColliderBase.h"

struct BoxCollider : public Collider
{
    static constexpr size_t CORNER_COUNT = 8;
    Vector3 HalfExtent;

    COLLIDER_TYPE GetType() const override { return COLLIDER_TYPE_BOX; }

    BOOL RayTest(const Vector3 rayStart, const Vector3 &rayDir, Vector3 *pOutNormal, float *tHit) override;

    Bounds GetBounds() const override
    {
        Vector3 mins = Position - HalfExtent;
        Vector3 maxs = Position + HalfExtent;
        return Bounds(mins, maxs);
    }

    void GetCorners(Vector3 pOutCorners[8]);

    BoxCollider(Vector3 halfExtent) : HalfExtent(halfExtent) {}
    ~BoxCollider() {}
};
