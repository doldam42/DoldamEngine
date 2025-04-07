#pragma once

#include "Collider.h"

class CharacterBody : public ICharacterBody
{
    IGameObject *m_pObject = nullptr;

    btCapsuleShape                 *m_pShape = nullptr;
    btPairCachingGhostObject       *m_pBody = nullptr;
    btKinematicCharacterController *m_pCharacter = nullptr;

  public:
    BOOL Initialize(btDynamicsWorld *pWorld, const Vector3 &startPosition, const float radius, const float height);

    btPairCachingGhostObject *Get() const { return m_pBody; }

    // Inherited via ICharacterBody
    void SetJumpSpeed(float speed) override;
    void Update(IGameObject *pObj) override;
    void Move(const Vector3 &dir) override;
    void Jump() override;
    BOOL OnGround() override;

    BOOL CanJump() override;
};
