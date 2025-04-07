#include "pch.h"
#include "Collider.h"
#include "RigidBody.h"

BOOL RigidBody::Initialize(IGameObject *pObj, ICollider *pCollider, float mass, float elasticity, float friction,
                           BOOL useGravity)
{
    m_pObject = pObj;
    Collider *pBase = (Collider *)pCollider;

    const Vector3 pos = pObj->GetPosition();
    btTransform   startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(btVector3(pos.x, pos.y, pos.z));

    btVector3 localInertia;
    localInertia.setZero();
    if (mass > 0.0f)
        pBase->m_pShape->calculateLocalInertia(mass, localInertia);
    
    btDefaultMotionState                    *myMotionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(btScalar(mass), myMotionState, pBase->m_pShape, localInertia);

    m_pBody = new btRigidBody(rbInfo);
    m_pBody->setFriction(friction);
    m_pBody->setDamping(1.0f - elasticity, 1.0f - elasticity);
    m_pBody->setUserPointer(this);
    return TRUE;
}

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
    m_pBody->applyCentralImpulse(btVector3(impulse.x, impulse.y, impulse.z));
}

void RigidBody::ApplyImpulseAngular(const Vector3 &impulse)
{
    m_pBody->applyTorqueImpulse(btVector3(impulse.x, impulse.y, impulse.z));
}

BOOL RigidBody::IsDynamic() { return !m_pBody->isStaticObject(); }