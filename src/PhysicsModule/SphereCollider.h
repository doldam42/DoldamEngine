#pragma once

#include "ColliderBase.h"

struct SphereCollider : public Collider
{
    float Radius;

  public:
    COLLIDER_TYPE GetType() const override { return COLLIDER_TYPE_SPHERE; }

    Bounds GetBounds() const override
    {
        Vector3 mins = Position - Vector3(Radius);
        Vector3 maxs = Position + Vector3(Radius);
        return Bounds(mins, maxs);
    }

    BOOL RayTest(const Vector3 rayStart, const Vector3 &rayDir, Vector3 *pOutNormal, float *tHit) override;

    SphereCollider(float radius) : Radius(radius) {}
    ~SphereCollider() {}
};
