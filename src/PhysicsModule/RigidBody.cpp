#include "pch.h"
#include "Collider.h"
#include "RigidBody.h"

void RigidBody::SetPosition(const Vector3 &pos) { m_position = pos; }

void RigidBody::SetRotation(const Quaternion &q) { m_rotation = q; }

void RigidBody::Update(IGameObject *pObj) 
{ 
    pObj->SetPosition(m_position.x, m_position.y, m_position.z); 
    pObj->SetRotation(m_rotation);
}

Vector3 RigidBody::GetVelocity() const { return Vector3(); }

void RigidBody::ApplyImpulseLinear(const Vector3 &impulse)
{
    applyCentralImpulse(btVector3(impulse.x, impulse.y, impulse.z));
}

void RigidBody::ApplyImpulseAngular(const Vector3 &impulse)
{
    applyTorqueImpulse(btVector3(impulse.x, impulse.y, impulse.z));
}

BOOL RigidBody::IsDynamic() { return !isStaticObject(); }

RigidBody::~RigidBody() { }