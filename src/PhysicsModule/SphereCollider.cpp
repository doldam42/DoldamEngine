#include "pch.h"

#include "SphereCollider.h"

BOOL SphereCollider::RayTest(const Vector3 rayStart, const Vector3 &rayDir, float *tHit)
{ 
	float t0, t1;
	if (RaySphere(rayStart, rayDir, Position, Radius, &t0, &t1))
	{
        *tHit = t0;
        return TRUE;
	}
    return FALSE;
}
