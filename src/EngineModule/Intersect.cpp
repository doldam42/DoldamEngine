#include "pch.h"

#include "Intersect.h"

BOOL RaySphere(const Vector3 &rayStart, const Vector3 &rayDir, const Vector3 &sphereCenter, const float sphereRadius,
               float *pOutT1, float *pOutT2)
{
    const Vector3 m = sphereCenter - rayStart;

    const float a = rayDir.Dot(rayDir);
    const float b = m.Dot(rayDir);
    const float c = m.Dot(m) - sphereRadius * sphereRadius;

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

BOOL SphereSphereStatic(const Sphere &a, const Sphere &b, const Vector3 &posA, const Vector3 &posB,
                        Vector3 *pOutContactPointA, Vector3 *pOutContactPointB, Vector3 *pOutNormal)
{
    Vector3 ab = posB - posA;

    const float radiusAB = a.Radius + b.Radius;
    const float lengthSquared = ab.LengthSquared();
    if (lengthSquared > (radiusAB * radiusAB))
    {
        return FALSE;
    }

    Vector3 normal = ab;
    normal.Normalize();

    *pOutContactPointA = posA + normal * a.Radius;
    *pOutContactPointB = posB - normal * b.Radius;

    *pOutNormal = normal;

    return TRUE;
}

BOOL SphereSphereDynamic(const Sphere &a, const Sphere &b, const Vector3 &posA, const Vector3 &posB,
                         const Vector3 &velA, const Vector3 &velB, const float dt, Vector3 *pOutContactPointA,
                         Vector3 *pOutContactPointB, Vector3 *pOutNormal, float *pOutToi)
{
    const Vector3 relativeVelocity = velA - velB;

    const Vector3 startPtA = posA;
    const Vector3 endPtA = posA + relativeVelocity * dt;
    const Vector3 rayDir = endPtA - startPtA;

    float t0 = 0;
    float t1 = 0;
    if (rayDir.LengthSquared() < 0.001f * 0.001f)
    {
        // Ray is too short, just check if already intersecting
        Vector3 ab = posB - posA;
        float   radius = a.Radius + b.Radius + 0.001f;
        if (ab.LengthSquared() > radius * radius)
        {
            return false;
        }
    }
    else if (!RaySphere(posA, rayDir, posB, a.Radius + b.Radius, &t0, &t1))
    {
        return false;
    }

    // Change from [0,1] range to [0,dt] range
    t0 *= dt;
    t1 *= dt;

    // If the collision is only in the past, then there's not future collision this frame
    if (t1 < 0.0f)
    {
        return false;
    }

    // Get the earliest positive time of impact
    float toi = (t0 < 0.0f) ? 0.0f : t0;

    // If the earliest collision is too far in the future, then there's no collision this frame
    if (toi > dt)
    {
        return false;
    }

    // Get the points on the respective points of collision and return true
    Vector3 newPosA = posA + velA * toi;
    Vector3 newPosB = posB + velB * toi;
    Vector3 ab = newPosB - newPosA;
    ab.Normalize();

    *pOutToi = toi;
    *pOutContactPointA = newPosA + ab * a.Radius;
    *pOutContactPointB = newPosB - ab * b.Radius;
    return true;
}

BOOL Intersect(PhysicsComponent *pA, PhysicsComponent *pB, const float dt, Contact *pOutContact)
{
    if (!pA || !pB)
        return FALSE;

    pOutContact->pA = pA;
    pOutContact->pB = pB;

    Vector3 posA = pA->GetPosition();
    Vector3 posB = pB->GetPosition();

    Vector3 velA = pA->GetVelocity();
    Vector3 velB = pB->GetVelocity();

    if (pA->m_pShape->GetType() == SHAPE_TYPE_SPHERE && pB->m_pShape->GetType() == SHAPE_TYPE_SPHERE)
    {
        Sphere *pSphereA = (Sphere *)pA->m_pShape;
        Sphere *pSphereB = (Sphere *)pB->m_pShape;

        float   timeOfImpact;
        Vector3 contactPointAWorldSpace;
        Vector3 contactPointBWorldSpace;
        Vector3 normal;
        if (SphereSphereDynamic(*pSphereA, *pSphereB, posA, posB, velA, velB, dt, &contactPointAWorldSpace,
                                &contactPointBWorldSpace, &normal, &timeOfImpact))
        {
            pA->Update(timeOfImpact);
            pB->Update(timeOfImpact);

            pOutContact->contactPointAWorldSpace = contactPointAWorldSpace;
            pOutContact->contactPointBWorldSpace = contactPointBWorldSpace;

            // Convert world space contacts to local space
            pOutContact->contactPointALocalSpace = pA->WorldSpaceToLocalSpace(contactPointAWorldSpace);
            pOutContact->contactPointBLocalSpace = pB->WorldSpaceToLocalSpace(contactPointBWorldSpace);

            pOutContact->normal = pA->GetPosition() - pB->GetPosition();
            pOutContact->normal.Normalize();

            pOutContact->timeOfImpact = timeOfImpact;

            // Unwind time step
            pA->Update(-timeOfImpact);
            pB->Update(-timeOfImpact);

            // Calculate the separation distance
            Vector3 ab = pB->GetPosition() - pA->GetPosition();
            float   r = ab.Length() - (pSphereA->Radius + pSphereB->Radius);

            pOutContact->separationDistance = r;

            return TRUE;
        }
        return FALSE;
    }
}
