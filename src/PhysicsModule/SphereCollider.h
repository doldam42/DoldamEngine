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

    SphereCollider(float radius) : Radius(radius) {}
    ~SphereCollider() {}
};
