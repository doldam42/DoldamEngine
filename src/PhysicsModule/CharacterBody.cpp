#include "pch.h"
#include "CharacterBody.h"

BOOL CharacterBody::Initialize(btDynamicsWorld *pWorld, const Vector3 &startPosition, const float radius,
                               const float height)
{
    m_pShape = new btCapsuleShape(btScalar(radius), btScalar(height));

    btTransform T;
    T.setIdentity();
    T.setOrigin(btVector3(startPosition.x, startPosition.y, startPosition.z));

    m_pBody = new btPairCachingGhostObject();
    m_pBody->setWorldTransform(T);
    m_pBody->setCollisionShape(m_pShape);
    m_pBody->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);

    m_pCharacter = new btKinematicCharacterController(m_pBody, m_pShape, 0.3f, btVector3(0.0f, 1.0f, 0.0f));

    pWorld->addCollisionObject(m_pBody, btBroadphaseProxy::CharacterFilter,
                               btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);

    pWorld->addAction(m_pCharacter);

    return TRUE;
}

void CharacterBody::SetJumpSpeed(float speed) { m_pCharacter->setJumpSpeed(speed); }

void CharacterBody::Update(IGameObject *pObj)
{ 
    const btVector3   &pos = m_pBody->getWorldTransform().getOrigin();
    pObj->SetPosition(pos.getX(), pos.getY(), pos.getZ());
}

void CharacterBody::Move(const Vector3 &dir) { 
    m_pCharacter->setWalkDirection(btVector3(dir.x, dir.y, dir.z));
}

void CharacterBody::Jump() { m_pCharacter->jump(); }

BOOL CharacterBody::OnGround() { return m_pCharacter->onGround(); }
