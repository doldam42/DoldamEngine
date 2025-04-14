#pragma once

#include "PhysicsBody.h"

class TriggerBody : public ITriggerBody, PhysicsBody
{
  public:
    btCollisionObject *Get() { return m_pBody; }

    BOOL Initialize(IGameObject *pObj, btCollisionShape *pShape, const Vector3 &pos);
    // Inherited via ITriggerBody
    void Update(IGameObject *pObj) override;
    BOOL IsCollisionEnter() override;
    BOOL IsCollisionStay() override;
    BOOL IsCollisionExit() override;
    void SetPosition(const Vector3 &pos) override;
    void SetRotation(const Quaternion &q) override;
    void SetActive(BOOL isActive) override;
    void Reset() override;
};
