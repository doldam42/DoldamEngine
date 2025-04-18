#include "pch.h"
#include "EllipsoidCollider.h"

BOOL EllipsoidCollider::RayTest(const Vector3 rayStart, const Vector3 &rayDir, Vector3 *pOutNormal, float *tHit)
{ 
    Vector3 normal;
	float hitt0, hitt1;
    if (RayEllipse(rayStart, rayDir, Position, MajorRadius, MinorRadius, &normal, &hitt0, &hitt1) && hitt0 > 0.0f)
	{
        *pOutNormal = normal;
        *tHit = hitt0;
        return TRUE;
	}
    return FALSE;
}
