#pragma once

BOOL RaySphere(const Vector3 &rayStart, const Vector3 &rayDir, const Vector3 &sphereCenter, const float sphereRadius,
               float *pOutT1, float *pOutT2);

BOOL RayTriangle(const Vector3 &rayStart, const Vector3 &rayDir, const Vector3 &v0, const Vector3 &v1,
                 const Vector3 &v2, float *pOuttHit);

BOOL RayCylinder(const Vector3 &rayStart, const Vector3 &rayDir, const Vector3 &p1, const Vector3 &p2,
                 const float radius, float *tHit);

BOOL RayEllipse(Vector3 rayStart, Vector3 rayDir, Vector3 center, float majorRadius, float minorRadius, float *pOutT1,
                float *pOutT2);

BOOL SphereSphereStatic(const float radiusA, const float radiusB, const Vector3 &posA, const Vector3 &posB,
                        Vector3 *pOutContactPointA, Vector3 *pOutContactPointB);

BOOL SphereSphereDynamic(const float radiusA, const float radiusB, const Vector3 &posA, const Vector3 &posB,
                         const Vector3 &velA, const Vector3 &velB, const float dt, Vector3 *pOutContactPointA,
                         Vector3 *pOutContactPointB, float *pOutToi);

BOOL SphereTriangleStatic(const Vector3 &sphereCenter, const float sphereRadius, const Vector3 &v0, const Vector3 &v1,
                          const Vector3 &v2);

BOOL SphereTriangleDynamic(const Vector3 &sphereCenter, const float sphereRadius, const Vector3 &sphereVelocity,
                           const float dt, const Vector3 &v0, const Vector3 &v1, const Vector3 &v2,
                           const Vector3 &normal, Vector3 *pOutContactPointA, Vector3 *pOutContactPointB,
                           float *pOutToi);

// 장축이 Y축이라 가정.
BOOL EllipseEllipseStatic(float majorRadiusA, float majorRadiusB, float minorRadiusA, float minorRadiusB, Vector3 posA,
                          Vector3 posB);

BOOL EllipseEllipseDynamic(float majorRadiusA, float majorRadiusB, float minorRadiusA, float minorRadiusB, Vector3 posA,
                           Vector3 posB, Vector3 velocity, const float dt, Vector3 *pOutNormal, float *pOutToi);

BOOL EllipseTriangleDynamic(Vector3 center, float majorRadius, float minorRadius, Vector3 velocity, float dt,
                            Vector3 v0, Vector3 v1, Vector3 v2, Vector3 normal, Vector3 *pOutNormal,
                            float *pOutToi);