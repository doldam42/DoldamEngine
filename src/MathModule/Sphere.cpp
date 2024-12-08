#include "pch.h"
#include "Sphere.h"

Bounds Sphere::GetBounds(const Vector3 &pos, const Quaternion &orient) const
{
    Bounds tmp;
    tmp.mins = Vector3(-Radius) + pos;
    tmp.maxs = Vector3(Radius) + pos;
    return tmp;
}

Bounds Sphere::GetBounds() const
{
    Bounds tmp;
    tmp.mins = Vector3(-Radius);
    tmp.maxs = Vector3(Radius);
    return tmp;
}