#include "pch.h"
#include "EllipsoidCollider.h"

BOOL EllipsoidCollider::RayTest(const Vector3 rayStart, const Vector3 &rayDir, float* tHit) 
{ 
	float hitt0, hitt1;
    if (RayEllipse(rayStart, rayDir, Position, MajorRadius, MinorRadius, &hitt0, &hitt1))
	{
        *tHit = hitt0;
        return TRUE;
	}
    return FALSE;
}
