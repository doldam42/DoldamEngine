#include "pch.h"

#include "Collisions.h"

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
    const float t1 = invA * (b - deltaRoot);
    const float t2 = invA * (b + deltaRoot);

    *pOutT1 = t1;
    *pOutT2 = t2;
    return TRUE;
}

// ref : https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
BOOL RayTriangle(const Vector3 &rayStart, const Vector3 &rayDir, const Vector3 &v0, const Vector3 &v1,
                 const Vector3 &v2, float *pOuttHit)
{
    constexpr float epsilon = MachineEpsilon;

    Vector3 edge1 = v1 - v0;
    Vector3 edge2 = v2 - v0;
    Vector3 ray_cross_e2 = rayDir.Cross(edge2);

    float det = edge1.Dot(ray_cross_e2);

    if (det > -epsilon && det < epsilon)
        return {}; // This ray is parallel to this triangle.

    float   inv_det = 1.0 / det;
    Vector3 s = rayStart - v0;
    float   u = inv_det * s.Dot(ray_cross_e2);

    if ((u < 0 && abs(u) > epsilon) || (u > 1 && abs(u - 1) > epsilon))
        return {};

    Vector3 s_cross_e1 = s.Cross(edge1);
    float   v = inv_det * rayDir.Dot(s_cross_e1);

    if ((v < 0 && abs(v) > epsilon) || (u + v > 1 && abs(u + v - 1) > epsilon))
        return FALSE;

    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = inv_det * edge2.Dot(s_cross_e1);

    if (t > epsilon) // ray intersection
    {
        *pOuttHit = t;
        return TRUE;
    }
    else // This means that there is a line intersection but not a ray intersection.
        return FALSE;
}

BOOL RayCylinder(const Vector3 &rayStart, const Vector3 &rayDir, const Vector3 &p1, const Vector3 &p2,
                 const float radius, float *tHit)
{
    Vector3 d = p2 - p1;
    d.Normalize();

    Vector3 m = rayStart - p1;
    Vector3 n = rayDir;
    Vector3 md = m - d * m.Dot(d);
    Vector3 nd = n - d * n.Dot(d);

    float a = nd.LengthSquared();
    float b = 2 * md.Dot(nd);
    float c = md.LengthSquared() - radius * radius;
    float disc = b * b - 4 * a * c;
    if (disc < 0)
        return false;

    float sqrtDisc = std::sqrt(disc);
    float t0 = (-b - sqrtDisc) / (2 * a);
    float t1 = (-b + sqrtDisc) / (2 * a);
    if (t0 > t1)
        std::swap(t0, t1);

    float h1 = (rayStart + rayDir * t0 - p1).Dot(d);
    float h2 = (rayStart + rayDir * t1 - p1).Dot(d);
    if ((h1 < 0 || h1 > (p2 - p1).LengthSquared()) && (h2 < 0 || h2 > (p2 - p1).LengthSquared()))
        return false;

    *tHit = (h1 >= 0 && h1 <= (p2 - p1).LengthSquared()) ? t0 : t1;
    return true;
}

BOOL RayEllipse(Vector3 rayStart, Vector3 rayDir, Vector3 center, float majorRadius, float minorRadius, float *pOutT1,
                float *pOutT2)
{
    const float scale = minorRadius / majorRadius;
    const float invScale = 1.0f / scale;

    rayStart.y *= scale;
    rayDir.y *= scale;
    center.y *= scale;

    float t0, t1;
    if (RaySphere(rayStart, rayDir, center, minorRadius, &t0, &t1))
    {
        *pOutT1 = t0;
        *pOutT2 = t1;
        return TRUE;
    }
    return FALSE;
}

BOOL SphereSphereStatic(const float radiusA, const float radiusB, const Vector3 &posA, const Vector3 &posB,
                        Vector3 *pOutContactPointA, Vector3 *pOutContactPointB)
{
    Vector3 ab = posB - posA;

    const float radiusAB = radiusA + radiusB;
    const float lengthSquared = ab.LengthSquared();
    if (lengthSquared > (radiusAB * radiusAB))
    {
        return FALSE;
    }

    Vector3 normal = ab;
    normal.Normalize();

    *pOutContactPointA = posA + normal * radiusA;
    *pOutContactPointB = posB - normal * radiusB;

    return TRUE;
}

BOOL SphereSphereDynamic(const float radiusA, const float radiusB, const Vector3 &posA, const Vector3 &posB,
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
    *pOutNormal = ab;
    *pOutContactPointA = newPosA + ab * radiusA;
    *pOutContactPointB = newPosB - ab * radiusB;
    return true;
}

inline float DistanceSquaredPointToSegment(const Vector3 &p, const Vector3 &a, const Vector3 &b)
{
    Vector3 ab = b - a;
    Vector3 ap = p - a;
    float   proj = ap.Dot(ab) / ab.Dot(ab);

    proj = std::clamp(proj, 0.0f, 1.0f);
    Vector3 closest = a + proj * ab;
    return (closest - p).LengthSquared();
}

BOOL SphereTriangleStatic(const Vector3 &sphereCenter, const float sphereRadius, const Vector3 &v0, const Vector3 &v1,
                          const Vector3 &v2)
{
    const float radiusSquared = sphereRadius * sphereRadius;

    Vector3 u0 = v1 - v0;
    Vector3 u1 = v2 - v0;
    Vector3 u2 = sphereCenter - v0;

    float dot00 = u0.Dot(u0);
    float dot01 = u0.Dot(u1);
    float dot02 = u0.Dot(u2);
    float dot11 = u1.Dot(u1);
    float dot12 = u1.Dot(u2);

    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    BOOL pointInTriangle = (u >= 0) && (v >= 0) && (u + v <= 1);

    // 1. 삼각형의 내부 점과의 충돌 검사
    if (pointInTriangle)
    {
        return TRUE;
    }

    // 2. 삼각형의 변과 원의 충돌 검사
    float distSquared01 = DistanceSquaredPointToSegment(sphereCenter, v0, v1);
    float distSquared12 = DistanceSquaredPointToSegment(sphereCenter, v1, v2);
    float distSquared20 = DistanceSquaredPointToSegment(sphereCenter, v2, v0);
    if (distSquared01 <= radiusSquared || distSquared12 <= radiusSquared || distSquared20 <= radiusSquared)
    {
        return TRUE;
    }

    // 3. 삼각형의 꼭짓점이 원 안에 있는지 확인
    if ((sphereCenter - v0).LengthSquared() <= radiusSquared || (sphereCenter - v1).LengthSquared() <= radiusSquared ||
        (sphereCenter - v2).LengthSquared() <= radiusSquared)
    {
        return true;
    }

    return false;
}

BOOL SphereTriangleDynamic(const Vector3 &sphereCenter, const float sphereRadius, const Vector3 &sphereVelocity,
                           const float dt, const Vector3 &v0, const Vector3 &v1, const Vector3 &v2,
                           const Vector3 &normal, Vector3 *pOutNormal, float *pOutToi)
{
    Vector3 dir = sphereVelocity;
    dir.Normalize();
    const float tMax = sphereVelocity.Length() * dt;

    // 삼각형의 3점을 중심으로 하는 원과의 충돌처리
    float t0, t1;
    if (RaySphere(sphereCenter, dir, v0, sphereRadius, &t0, &t1) && t0 < tMax && t1 >= 0.0f)
    {
        const Vector3 hitPoint = sphereCenter + dir * t0;
        const Vector3 n = hitPoint - v0;

        *pOutToi = t0 * dt;
        *pOutNormal = n;
        return TRUE;
    }
    if (RaySphere(sphereCenter, dir, v1, sphereRadius, &t0, &t1) && t0 < tMax && t1 >= 0.0f)
    {
        const Vector3 hitPoint = sphereCenter + dir * t0;
        const Vector3 n = hitPoint - v1;

        *pOutToi = t0 * dt;
        *pOutNormal = n;
        return TRUE;
    }
    if (RaySphere(sphereCenter, dir, v2, sphereRadius, &t0, &t1) && t0 < tMax && t1 >= 0.0f)
    {
        const Vector3 hitPoint = sphereCenter + dir * t0;
        const Vector3 n = hitPoint - v2;

        *pOutToi = t0 * dt;
        *pOutNormal = n;
        return TRUE;
    }

    // 삼각형의 변을 변환한 원통과의 충돌처리
    if (RayCylinder(sphereCenter, dir, v0, v1, sphereRadius, &t0) && t0 < tMax && t1 >= 0.0f)
    {
        const Vector3 hitPoint = sphereCenter + dir * t0;
        const Vector3 d = v1 - v0;
        const float   t = (hitPoint - v0).Dot(d) / d.LengthSquared();
        const Vector3 p = v0 + d * t;
        const Vector3 n = hitPoint - p;

        *pOutToi = t0 * dt;
        *pOutNormal = n;
        return TRUE;
    }
    if (RayCylinder(sphereCenter, dir, v1, v2, sphereRadius, &t0) && t0 < tMax && t1 >= 0.0f)
    {
        const Vector3 hitPoint = sphereCenter + dir * t0;
        const Vector3 d = v2 - v1;
        const float   t = (hitPoint - v1).Dot(d) / d.LengthSquared();
        const Vector3 p = v1 + d * t;
        const Vector3 n = hitPoint - p;

        *pOutToi = t0 * dt;
        *pOutNormal = n;
        return TRUE;
    }
    if (RayCylinder(sphereCenter, dir, v2, v0, sphereRadius, &t0) && t0 < tMax && t1 >= 0.0f)
    {
        const Vector3 hitPoint = sphereCenter + dir * t0;
        const Vector3 d = v0 - v2;
        const float   t = (hitPoint - v2).Dot(d) / d.LengthSquared();
        const Vector3 p = v2 + d * t;
        const Vector3 n = hitPoint - p;

        *pOutToi = t0 * dt;
        *pOutNormal = n;
        return TRUE;
    }

    // normal 방향으로 이동한 삼각형과의 충돌처리
    if (RayTriangle(sphereCenter, dir, v0 + normal * sphereRadius, v1 + normal * sphereRadius,
                    v2 + normal * sphereRadius, &t0) && t0 < tMax)
    {
        *pOutToi = t0 * dt;
        *pOutNormal = normal;
        return TRUE;
    }
    return FALSE;
}

BOOL EllipseEllipseStatic(float majorRadiusA, float majorRadiusB, float minorRadiusA, float minorRadiusB, Vector3 posA,
                          Vector3 posB)
{
    const float scale = minorRadiusA / majorRadiusA;
    const float invScale = 1.0f / scale;

    minorRadiusB *= scale;
    majorRadiusB *= scale;
    posA.y *= scale;
    posB.y *= scale;

    Vector3 dir = posB - posA;
    dir.Normalize();

    float dummyhitt0, dummyhitt1;
    return RayEllipse(posA, dir, posB, majorRadiusB + minorRadiusA, minorRadiusB + minorRadiusA, &dummyhitt0,
                      &dummyhitt1);
}

BOOL EllipseEllipseDynamic(float majorRadiusA, float majorRadiusB, float minorRadiusA, float minorRadiusB, Vector3 posA,
                           Vector3 posB, Vector3 velocity, const float dt, Vector3 *pOutNormal,
                           float *pOutToi)
{
    const float scale = minorRadiusA / majorRadiusA;
    const float invScale = 1.0f / scale;

    Vector3 dir = velocity;
    dir.Normalize();

    // 장축(Y축)을 기준으로 Scale
    majorRadiusB *= scale;
    dir.y *= scale;
    posA.y *= scale;
    posB.y *= scale;

    float t0, t1;
    if (!RayEllipse(posA, dir, posB, majorRadiusB + minorRadiusA, minorRadiusB + minorRadiusA, &t0, &t1) || t1 < 0.0f)
    {
        return FALSE;
    }

    /*Vector3 tmp = posA + dir * t0;
    Vector3 contactPoint = Vector3::Lerp(tmp, posB, minorRadiusA / (tmp - posB).Length());*/

    // 원상 복구
    /*contactPoint.y *= invScale;*/
    majorRadiusB *= invScale;
    dir.y *= invScale;
    posA.y *= invScale;
    posB.y *= invScale;

    Vector3 collisionPoint = posA + dir * t0;

    t0 *= dt;
    t1 *= dt;

    float toi = (t0 < 0.0f) ? 0.0f : t0;
    if (toi > dt)
    {
        return false;
    }

    const float majorRS = majorRadiusB * majorRadiusB;
    const float minorRS = minorRadiusB * minorRadiusB;
    Vector3     n = (collisionPoint - posB) / Vector3(minorRS, majorRS, minorRS);
    n.Normalize();

    *pOutNormal = n;
    *pOutToi = toi;
    return TRUE;
}

BOOL EllipseTriangleDynamic(Vector3 center, float majorRadius, float minorRadius, Vector3 velocity, float dt,
                            Vector3 v0, Vector3 v1, Vector3 v2, Vector3 normal, Vector3 *pOutNormal, float *pOutToi)
{
    const float scale = minorRadius / majorRadius;
    const float invScale = 1.0f / scale;

    v0.y *= scale;
    v1.y *= scale;
    v2.y *= scale;
    center.y *= scale;
    velocity.y *= scale;
    normal.y *= scale;

    float   t0;
    Vector3 n;
    if (SphereTriangleDynamic(center, minorRadius, velocity, dt, v0, v1, v2, normal, &n, &t0))
    {
        t0 *= dt;

        float toi = (t0 < 0.0f) ? 0.0f : t0;
        if (toi > dt)
        {
            return false;
        }


        *pOutNormal = n;
        *pOutToi = t0;
        n.y *= invScale;
        return TRUE;
    }

    return FALSE;
}
