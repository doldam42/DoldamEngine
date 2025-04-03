#pragma once

#include "Contact.h"
BOOL SphereSphere(const float radiusA, const float radiusB, const Vector3 &posA, const Vector3 &posB,
                  const Vector3 &velA, const Vector3 &velB, const float dt, float *toi, Contact *contact);

BOOL SphereTriangle(const Vector3 &sphereCenter, const float sphereRadius, const Vector3 &sphereVelocity,
                    const float dt, const Vector3 &v0, const Vector3 &v1, const Vector3 &v2, const Vector3 &normal,
                    float *toi, Contact *contact);
