#include "pch.h"
#include "BoxCollider.h"

BOOL BoxCollider::RayTest(const Vector3 rayStart, const Vector3 &rayDir, Vector3 *pOutNormal, float *tHit)
{ 
    Vector3 normal;
    float   hitt;
    if (RayBox(rayStart, rayDir, Position, HalfExtent, Rotation, &normal, &hitt))
    {
        *tHit = hitt;
        *pOutNormal = normal;
        return TRUE;
    }
    return FALSE;
}

void BoxCollider::GetCorners(Vector3 pOutCorners[8])
{
    Bounds b(-HalfExtent, HalfExtent);

    b.GetCorners(pOutCorners);
    for (int i = 0; i < CORNER_COUNT; i++)
    {
        pOutCorners[i] = Vector3::Transform(pOutCorners[i], Rotation) + Position;
    }
}
