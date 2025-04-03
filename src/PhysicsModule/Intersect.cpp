#include "Intersect.h"
#include "pch.h"

BOOL SphereSphere(const float radiusA, const float radiusB, const Vector3 &posA, const Vector3 &posB,
                  const Vector3 &velA, const Vector3 &velB, const float dt, float *toi, Contact *contact)
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
        float   radius = radiusA + radiusB + 0.001f;
        if (ab.LengthSquared() > radius * radius)
        {
            return false;
        }
    }
    else if (!RaySphere(posA, rayDir, posB, radiusA + radiusB, &t0, &t1))
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
    float _toi = (t0 < 0.0f) ? 0.0f : t0;

    // If the earliest collision is too far in the future, then there's no collision this frame
    if (_toi > dt)
    {
        return false;
    }

    // Get the points on the respective points of collision and return true
    Vector3 newPosA = posA + velA * _toi;
    Vector3 newPosB = posB + velB * _toi;
    Vector3 ab = newPosB - newPosA;

    contact->penetration = radiusA + radiusB - ab.Length();
    contact->position = newPosA + ab * 0.5f;
    contact->normal = ab;
    contact->normal.Normalize();
    *toi = _toi;

    return true;
}

BOOL SphereTriangle(const Vector3 &sphereCenter, const float sphereRadius, const Vector3 &sphereVelocity,
                    const float dt, const Vector3 &v0, const Vector3 &v1, const Vector3 &v2, const Vector3 &normal,
                    float *toi, Contact *contact)
{
    //float   t0, t1;
    //Vector3 dir = sphereVelocity;
    //dir.Normalize();
    //const float tMax = sphereVelocity.Length() * dt;

    //// 삼각형의 변을 변환한 원통과의 충돌처리
    //if (RayCylinder(sphereCenter, dir, v0, v1, sphereRadius, &t0) && t0 < tMax && t1 >= 0.0f)
    //{
    //    const Vector3 hitPoint = sphereCenter + dir * t0;

    //    Vector3 p;
    //    FootOfAltitude(v0, v1, hitPoint, &p);

    //    Vector3 normal = hitPoint - p;
    //    normal.Normalize();

    //    contact->position = p;
    //    
    //    *toi = t0 * dt;
    //    return TRUE;
    //}
    //if (RayCylinder(sphereCenter, dir, v1, v2, sphereRadius, &t0) && t0 < tMax && t1 >= 0.0f)
    //{
    //    const Vector3 hitPoint = sphereCenter + dir * t0;

    //    Vector3 p;
    //    FootOfAltitude(v1, v2, hitPoint, &p);

    //    Vector3 normal = hitPoint - p;
    //    normal.Normalize();

    //    contact->position = p;

    //    *toi = t0 * dt;
    //    return TRUE;
    //}
    //if (RayCylinder(sphereCenter, dir, v2, v0, sphereRadius, &t0) && t0 < tMax && t1 >= 0.0f)
    //{
    //    const Vector3 hitPoint = sphereCenter + dir * t0;

    //    Vector3 p;
    //    FootOfAltitude(v2, v0, hitPoint, &p);

    //    Vector3 normal = hitPoint - p;
    //    normal.Normalize();

    //    contact->position = p;

    //    *toi = t0 * dt;
    //    return TRUE;
    //}

    //// normal 방향으로 이동한 삼각형과의 충돌처리
    //if (RayTriangle(sphereCenter, dir, v0 + normal * sphereRadius, v1 + normal * sphereRadius,
    //                v2 + normal * sphereRadius, &t0) &&
    //    t0 < tMax)
    //{
    //    const Vector3 hitPoint = sphereCenter + dir * t0;

    //    *pOutContactPointA = hitPoint + n * sphereRadius;
    //    *pOutContactPointB = p;

    //    *pOutToi = t0 * dt;

    //    return TRUE;
    //}

    //// 삼각형의 3점을 중심으로 하는 원과의 충돌처리
    //if (RaySphere(sphereCenter, dir, v0, sphereRadius, &t0, &t1) && t0 < tMax && t1 >= 0.0f)
    //{
    //    *pOutContactPointA = sphereCenter + dir * (t0 + sphereRadius);
    //    *pOutContactPointB = v0;

    //    *pOutToi = (t0 < 0.0f) ? 0.0f : t0;
    //    return TRUE;
    //}
    //if (RaySphere(sphereCenter, dir, v1, sphereRadius, &t0, &t1) && t0 < tMax && t1 >= 0.0f)
    //{
    //    *pOutContactPointA = sphereCenter + dir * (t0 + sphereRadius);
    //    *pOutContactPointB = v1;

    //    *pOutToi = (t0 < 0.0f) ? 0.0f : t0;
    //    return TRUE;
    //}
    //if (RaySphere(sphereCenter, dir, v2, sphereRadius, &t0, &t1) && t0 < tMax && t1 >= 0.0f)
    //{
    //    *pOutContactPointA = sphereCenter + dir * (t0 + sphereRadius);
    //    *pOutContactPointB = v2;

    //    *pOutToi = (t0 < 0.0f) ? 0.0f : t0;
    //    return TRUE;
    //}

    return FALSE;
}
