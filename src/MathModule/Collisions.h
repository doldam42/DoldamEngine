#pragma once

BOOL RaySphere(const Vector3 &rayStart, const Vector3 &rayDir, const Vector3 &sphereCenter, const float sphereRadius, Vector3* pOutNormal,
               float *pOutT1, float *pOutT2);

BOOL RayTriangle(const Vector3 &rayStart, const Vector3 &rayDir, const Vector3 &v0, const Vector3 &v1,
                 const Vector3 &v2, float *pOuttHit);

BOOL RayCylinder(const Vector3 &rayStart, const Vector3 &rayDir, const Vector3 &p1, const Vector3 &p2,
                 const float radius, float *tHit);

BOOL RayBox(const Vector3 &rayStart, const Vector3 &rayDir, const Vector3 &boxPos, const Vector3 &boxExtent,
            const Quaternion &boxRot, Vector3 *pOutNormal, float *tHit);

BOOL RayEllipse(Vector3 rayStart, Vector3 rayDir, Vector3 center, float majorRadius, float minorRadius,
                Vector3 *pOutNormal, float *pOutT1,
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
                          Vector3 posB, Vector3 *pOutContactPointA, Vector3 *pOutContactPointB);

BOOL EllipseEllipseDynamic(float majorRadiusA, float majorRadiusB, float minorRadiusA, float minorRadiusB, Vector3 posA,
                           Vector3 posB, const Vector3& velA, const Vector3& velB, const float dt, Vector3 *pOutNormal, float *pOutToi);

BOOL EllipseTriangleDynamic(Vector3 center, float majorRadius, float minorRadius, Vector3 velocity, float dt,
                            Vector3 v0, Vector3 v1, Vector3 v2, Vector3 normal, Vector3 *pOutNormal,
                            float *pOutToi);

BOOL BoxBoxStatic(const Vector3 &halfExtentA, const Vector3 &halfExtentB, const Quaternion &rotA, const Quaternion &rotB, const Vector3 &posA, const Vector3 &posB, Vector3 *pOutContactPointA,
                  Vector3 *pOutContactPointB);

BOOL BoxBoxDynamic(const Vector3 &halfExtentA, const Vector3 &halfExtentB, const Quaternion &rotA,
                   const Quaternion &rotB, const Vector3 &posA, const Vector3 &posB, const Vector3 &velA,
                   const Vector3 &velB, Vector3 *pOutContactPointA, Vector3 *pOutContactPointB, float *pOutToi);

BOOL SphereBoxStatic(const float sphereRadius, const Vector3 &spherePos, const Vector3 &obbHalfExtent,
                     const Quaternion &obbRot, const Vector3 &boxPos, Vector3* pOutContactPointA, Vector3* pOutContactPointB);

BOOL BoxEllipseStatic(Vector3 obbHalfExtent, const Quaternion &obbRot, Vector3 obbPos, float majorRadius,
                      float minorRadius, Vector3 ellipsePos, Vector3 *pOutContactPointA, Vector3 *pOutContactPointB);
