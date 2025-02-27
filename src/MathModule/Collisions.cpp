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
    *pOutT1 = invA * (b - deltaRoot);
    *pOutT2 = invA * (b + deltaRoot);

    return TRUE;
}

// ref : https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
BOOL RayTriangle(const Vector3 &rayStart, const Vector3 &rayDir, const Vector3 &v0, const Vector3 &v1,
                 const Vector3 &v2, const Vector3 &vertexNormal, Vector3 *pOutNormal)
{
    constexpr float epsilon = std::numeric_limits<float>::epsilon();

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
        // R = I-2(I*N)N
        *pOutNormal = rayDir - 2.0f * rayDir.Dot(vertexNormal) * vertexNormal;
        return TRUE;
    }
    else // This means that there is a line intersection but not a ray intersection.
        return FALSE;
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
