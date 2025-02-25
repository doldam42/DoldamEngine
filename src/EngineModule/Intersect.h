#pragma once

#include "Contact.h"

BOOL RaySphere(const Vector3 &rayStart, const Vector3 &rayDir, const Vector3 &sphereCenter, const float sphereRadius,
               float *pOutT1, float *pOutT2);

BOOL SphereSphereStatic(const Sphere &a, const Sphere &b, const Vector3 &posA, const Vector3 &posB,
                        Vector3 *pOutContactPointA, Vector3 *pOutContactPointB, Vector3 *pOutNormal);

BOOL SphereSphereDynamic(const Sphere &a, const Sphere &b, const Vector3 &posA, const Vector3 &posB,
                         const Vector3 &velA, const Vector3 &velB, const float dt, Vector3 *pOutContactPointA,
                         Vector3 *pOutContactPointB, Vector3 *pOutNormal, float *pOutToi);

BOOL Intersect(PhysicsComponent *pA, PhysicsComponent *pB, const float dt, Contact *pOutContact);