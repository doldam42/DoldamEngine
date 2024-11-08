#include "pch.h"

#include "Box.h"
#include "Sphere.h"

#include "Collider.h"

BOOL CheckCollision(const Box &box, const Sphere &sphere, Contact *pOutContact)
{
    // 여기서 문제가 있음...
    Vector3 closestPoint = box.GetClosestPoint(sphere.Center);

    Vector3 collisionNormal = closestPoint - sphere.Center;
    float   penetrationDepth = sphere.Radius - collisionNormal.Length();

    if (penetrationDepth < 1e-4f)
        return FALSE;

    collisionNormal.Normalize();

    /*pOutContact->pShapeA = &box;
    pOutContact->pShapeB = &sphere;*/

    pOutContact->worldSpaceA = closestPoint;
    pOutContact->worldSpaceB = sphere.Center + collisionNormal * sphere.Radius;

    pOutContact->normal = collisionNormal;

    return TRUE;
}

BOOL CheckCollision(const Sphere &a, const Sphere &b, Contact *pOutContact)
{
    Vector3 ab = b.Center - a.Center;

    const float radiusAB = a.Radius + b.Radius;
    const float lengthSquared = ab.LengthSquared();
    if (lengthSquared > (radiusAB * radiusAB))
    {
        return FALSE;
    }

    Vector3 normal = ab;
    normal.Normalize();

    /*pOutContact->pShapeA = &a;
    pOutContact->pShapeB = &b;*/

    pOutContact->worldSpaceA = a.Center + normal * a.Radius;
    pOutContact->worldSpaceB = b.Center + normal * b.Radius;

    pOutContact->normal = normal;

    return TRUE;
}
