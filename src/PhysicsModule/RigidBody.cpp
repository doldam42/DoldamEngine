#include "pch.h"
#include "RigidBody.h"

void RigidBody::Update(IGameObject *pObj) {}

Vector3 RigidBody::GetVelocity() const { return Vector3(); }

void RigidBody::ApplyImpulseLinear(const Vector3 &impulse) {}

void RigidBody::ApplyImpulseAngular(const Vector3 &impulse) {}

void RigidBody::SetActive(BOOL isActive) {}

void RigidBody::Reset() {}
