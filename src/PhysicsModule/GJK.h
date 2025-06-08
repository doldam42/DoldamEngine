#pragma once

#include "ColliderBase.h"

bool GJK_DoesIntersect(const Collider* pA, const Collider* pB);
bool GJK_DoesIntersect(const Collider *pA, const Collider *pB, const float bias, Vector3 *ptOnA, Vector3 *ptOnB);
void GJK_ClosestPoints(const Collider *bodyA, const Collider *bodyB, Vector3 *ptOnA, Vector3 *ptOnB);