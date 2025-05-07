#pragma once

#include "ColliderBase.h"

struct EllipsoidCollider : public Collider
{
    float MajorRadius; // Y axis
    float MinorRadius; // X-Z axis

    COLLIDER_TYPE GetType() const override { return COLLIDER_TYPE_ELLIPSOID; }

    BOOL RayTest(const Vector3 rayStart, const Vector3 &rayDir, Vector3 *pOutNormal, float *tHit) override;

    Bounds GetBounds() const override
    {
        Vector3 extent = Vector3(MinorRadius, MajorRadius, MinorRadius);
        Vector3 mins = Position - extent;
        Vector3 maxs = Position + extent;
        return Bounds(mins, maxs);
    }

    EllipsoidCollider(float majorRadius, float minorRadius) : MajorRadius(majorRadius), MinorRadius(minorRadius) {}
    ~EllipsoidCollider() {}
};
