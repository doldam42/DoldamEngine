#pragma once

class PhysicsBody : public IPhysicsBody
{
  protected:
    btCollisionObject *m_pBody;
    IGameObject       *m_pGameObject;

  public:
    BOOL IsCollision = FALSE;
    BOOL IsPrevCollision = FALSE;

  public:
    void Update(IGameObject *pObj) override;

    BOOL IsCollisionEnter() override;
    BOOL IsCollisionStay() override;
    BOOL IsCollisionExit() override;
    void SetPosition(const Vector3 &pos) override;
    void SetRotation(const Quaternion &q) override;
    void SetActive(BOOL isActive) override;
    void Reset() override;
};
