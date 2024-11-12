#include "pch.h"

#include "Shape.h"
#include "Sphere.h"
#include "PhysicsComponent.h"

#include "Intersections.h"

BOOL RaySphere(const Vector3& rayStart, const Vector3& rayDir, const Vector3& sphereCenter, const float sphereRadius, float* pOutT1, float* pOutT2)
{
    const Vector3 m = sphereCenter - rayStart;
    const float   a = rayDir.Dot(rayDir);
    const float   b = m.Dot(rayDir);
    const float   c = m.Dot(m) - sphereRadius * sphereRadius;

    const float delta = b * b - a * c;
    const float invA = 1.0f / a;

    if (delta < 0)
    {
        return FALSE;
    }

    const float deltaRoot = sqrtf(delta);
    *pOutT1 = invA * (b - deltaRoot);
    *pOutT2 = invA * (b + deltaRoot);

    return TRUE;
}

BOOL SphereSphereDynamic(const ShapeSphere* shapeA, const ShapeSphere* shapeB, const Vector3& posA, const Vector3& posB, const Vector3& velA, const Vector3& velB, const float dt, Vector3* pOutPointOnA, Vector3* pOutPointOnB, float* pOutToi)
{
    const Vector3 relativeVelocity = velA - velB;

	const Vector3 startPointA = posA;
    const Vector3 endPointA = posA + relativeVelocity * dt;
    const Vector3 rayDir = endPointA - startPointA;

    float t0 = 0;
    float t1 = 0;
    if (rayDir.LengthSquared() < 0.001f * 0.001f)
    {
        // Ray is too short, just check if already intersect
        Vector3 ab = posB - posA;
        float   radius = shapeA->Radius + shapeB->Radius + 0.001f;
        if (ab.LengthSquared() > radius * radius)
        {
            return FALSE;
        }
    }
    else if (!RaySphere(posA, rayDir, posB, shapeA->Radius + shapeB->Radius, &t0, &t1))
    {
        return FALSE;
    }

    // Change from [0,1] range to [0,dt] range;
    t0 *= dt;
    t1 *= dt;

    // if the collision is only in the pas, then there's not future collision this frame
    if (t1 < 0.0f)
    {
        return FALSE;
    }

    // Get the earliest positive time of impact
    float toi = (t0 < 0.0f) ? 0.0f : t0;

    // if the earlest collision is too far in the future, then there's no collision this frame
    if (toi > dt)
    {
        return FALSE;
    }

    // Get the points on the respective points of collision and return true;
    Vector3 newPosA = posA + velA * toi;
    Vector3 newPosB = posB + velB * toi;
    Vector3 ab = newPosB - newPosA;
    ab.Normalize();

    *pOutToi = toi;
    *pOutPointOnA = newPosA + ab * shapeA->Radius;
    *pOutPointOnB = newPosB + ab * shapeB->Radius;

    return TRUE;
}
BOOL Intersect(PhysicsComponent *pA, PhysicsComponent *pB, const float dt, Contact *pOutContact)
{ 
	pOutContact->pA = pA;
    pOutContact->pB = pB;

	if (pA->m_pShape->GetType() == SHAPE_TYPE_SPHERE && pB->m_pShape->GetType() == SHAPE_TYPE_SPHERE)
	{
        const ShapeSphere *sphereA = (const ShapeSphere *)pA->m_pShape;
        const ShapeSphere *sphereB = (const ShapeSphere *)pB->m_pShape;

		Vector3 posA = pA->m_position;
        Vector3 posB = pB->m_position;

		Vector3 velA = pA->m_linearVelocity;
        Vector3 velB = pB->m_linearVelocity;

        if (SphereSphereDynamic(sphereA, sphereB, posA, posB, velA, velB, dt, &pOutContact->worldSpaceA, &pOutContact->worldSpaceB, &pOutContact->timeOfImpact))
        {
            pA->Update()
        }
	}
}
