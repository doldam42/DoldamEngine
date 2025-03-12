#include "pch.h"
#include "Plane.h"

Plane::Plane(const Vector3 &InPoint1, const Vector3 &InPoint2, const Vector3 &InPoint3)
{
    Vector3 v1 = InPoint2 - InPoint1;
    Vector3 v2 = InPoint3 - InPoint1;

    normal = v1.Cross(v2);
    normal.Normalize();
    d = -normal.Dot(InPoint1);
}

Plane::Plane(const Vector4 &InVector4)
{
    normal = Vector3(InVector4.x, InVector4.y, InVector4.z);
    d = InVector4.w;
    Normalize();
}

void Plane::Normalize()
{
    float squaredSize = normal.LengthSquared();

    if (abs(squaredSize - 1.0f) <= 1e-4f)
    {
        return;
    }

    float invLength = 1 / sqrtf(squaredSize);
    normal *= invLength;
    d *= invLength;
}