#include "pch.h"
#include "PhysicsBody.h"

void PhysicsBody::Update(IGameObject *pObj)
{
    IsPrevCollision = IsCollision;
    IsCollision = FALSE;
}

BOOL PhysicsBody::IsCollisionEnter() { return (!IsPrevCollision && IsCollision); }

BOOL PhysicsBody::IsCollisionStay() { return (IsPrevCollision && IsCollision); }

BOOL PhysicsBody::IsCollisionExit() { return (IsPrevCollision && !IsCollision); }

void PhysicsBody::SetPosition(const Vector3 &pos) 
{ 
	btTransform &tr = m_pBody->getWorldTransform();
    tr.setOrigin(btVector3(pos.x, pos.y, pos.z));
}

void PhysicsBody::SetRotation(const Quaternion &q)
{
    btTransform &tr = m_pBody->getWorldTransform();
    tr.setRotation(btQuaternion(q.x, q.y, q.z, q.w));
}

void PhysicsBody::SetActive(BOOL isActive) 
{
    if (isActive)
    {
        m_pBody->forceActivationState(ACTIVE_TAG);
        m_pBody->activate();
    }
    else
    {
        m_pBody->setActivationState(DISABLE_SIMULATION);
    }
}

void PhysicsBody::Reset()
{
    btTransform T;
    T.setIdentity();
    m_pBody->setWorldTransform(T);
}
