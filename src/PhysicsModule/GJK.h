#pragma once

#include "RigidBody.h"

bool GJK_DoesIntersect(const RigidBody *bodyA, const RigidBody *bodyB);
bool GJK_DoesIntersect(const RigidBody *bodyA, const RigidBody *bodyB, const float bias, Vector3 &ptOnA, Vector3 &ptOnB);
void GJK_ClosestPoints(const RigidBody *bodyA, const RigidBody *bodyB, Vector3 &ptOnA, Vector3 &ptOnB);