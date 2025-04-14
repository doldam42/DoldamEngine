#include "pch.h"

#include "Collider.h"

#include "TriggerBody.h"

BOOL TriggerBody::Initialize(IGameObject *pObj, btCollisionShape *pShape, const Vector3 &pos)
{
    btTransform T;
    T.setIdentity();
    T.setOrigin(btVector3(pos.x, pos.y, pos.z));

    m_pBody = new btPairCachingGhostObject();
    m_pBody->setWorldTransform(T);
    m_pBody->setCollisionShape(pShape);

    m_pBody->setUserPointer(this);
}

void TriggerBody::Update(IGameObject *pObj) { PhysicsBody::Update(pObj); }

BOOL TriggerBody::IsCollisionEnter() { return PhysicsBody::IsCollisionEnter(); }

BOOL TriggerBody::IsCollisionStay() { return PhysicsBody::IsCollisionStay(); }

BOOL TriggerBody::IsCollisionExit() { return PhysicsBody::IsCollisionExit(); }

void TriggerBody::SetPosition(const Vector3 &pos) { PhysicsBody::SetPosition(pos); }

void TriggerBody::SetRotation(const Quaternion &q) { PhysicsBody::SetRotation(q); }

void TriggerBody::SetActive(BOOL isActive) { PhysicsBody::SetActive(isActive); }

void TriggerBody::Reset() { PhysicsBody::Reset(); }
