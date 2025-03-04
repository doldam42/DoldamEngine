#pragma once

BOOL RaySphere(const Vector3 &rayStart, const Vector3 &rayDir, const Vector3 &sphereCenter, const float sphereRadius,
               float *pOutT1, float *pOutT2);

BOOL RayTriangle(const Vector3 &rayStart, const Vector3 &rayDir, const Vector3 &v0, const Vector3 &v1,
                 const Vector3 &v2, const Vector3 &vertexNormal);

BOOL RayCylinder(const Vector3 &rayStart, const Vector3 &rayDir, const Vector3 &p1, const Vector3 &p2,
                 const float radius, float *tHit);

BOOL SphereSphereStatic(const float radiusA, const float radiusB, const Vector3 &posA, const Vector3 &posB,
                        Vector3 *pOutContactPointA, Vector3 *pOutContactPointB);

BOOL SphereSphereDynamic(const float radiusA, const float radiusB, const Vector3 &posA, const Vector3 &posB,
                         const Vector3 &velA, const Vector3 &velB, const float dt, Vector3 *pOutContactPointA,
                         Vector3 *pOutContactPointB, float *pOutToi);

BOOL SphereTriangleStatic(const Vector3 &sphereCenter, const float sphereRadius, const Vector3 &v0, const Vector3 &v1,
                          const Vector3 &v2);

BOOL SphereTriangleDynamic(const Vector3 &sphereCenter, const float sphereRadius, const Vector3 &sphereVelocity,
                           const float dt, const Vector3 &v0, const Vector3 &v1, const Vector3 &v2,
                           const Vector3 &normal, Vector3 *pOutSlide, Vector3 *pOutReflect, float *pOutToi);