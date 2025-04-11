#include "pch.h"
#include "Collider.h"
#include "RigidBody.h"

void RigidBody::SetPosition(const Vector3 &pos) 
{
    btTransform& T = getWorldTransform();
    T.setOrigin(btVector3(pos.x, pos.y, pos.z));
    getMotionState()->setWorldTransform(T);
}

void RigidBody::SetRotation(const Quaternion &q)
{
    btTransform& T = getWorldTransform();
    T.setRotation(btQuaternion(q.x, q.y, q.z, q.w));
    getMotionState()->setWorldTransform(T);
}

void RigidBody::SetPositionInternal(const Vector3 &pos) { m_position = pos; }

void RigidBody::SetRotationInternal(const Quaternion &q) { m_rotation = q; }

void RigidBody::Update(IGameObject *pObj) 
{ 
    pObj->SetPosition(m_position.x, m_position.y, m_position.z); 
    pObj->SetRotation(m_rotation);
}

Vector3 RigidBody::GetVelocity() const
{
    const btVector3 &v = getLinearVelocity();
    return Vector3(v.x(), v.y(), v.z());
}

void RigidBody::ApplyImpulseLinear(const Vector3 &impulse)
{
    applyCentralImpulse(btVector3(impulse.x, impulse.y, impulse.z));
}

void RigidBody::ApplyImpulseAngular(const Vector3 &impulse)
{
    applyTorqueImpulse(btVector3(impulse.x, impulse.y, impulse.z));
}

BOOL RigidBody::IsDynamic() { return !isStaticObject(); }

void RigidBody::SetActive(BOOL isActive)
{
    if (isActive)
    {
        forceActivationState(ACTIVE_TAG);
        activate();
    }
    else
    {
        forceActivationState(DISABLE_SIMULATION);
    }
}

void RigidBody::Reset() 
{ 
    m_position = Vector3::Zero;
    m_rotation = Quaternion::Identity;
    
    btTransform T;
    T.setIdentity();
    setWorldTransform(T);
    getMotionState()->setWorldTransform(T);
    setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
    setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
    clearForces();
}

RigidBody::~RigidBody() {}