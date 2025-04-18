#include "pch.h"

#include "SphereCollider.h"

BOOL SphereCollider::RayTest(const Vector3 rayStart, const Vector3 &rayDir, Vector3 *pOutNormal, float *tHit)
{ 
	Vector3 normal;
	float t0, t1;
    if (RaySphere(rayStart, rayDir, Position, Radius, &normal, & t0, &t1))
	{
        *pOutNormal = normal;
        *tHit = t0;
        return TRUE;
	}
    return FALSE;
}
