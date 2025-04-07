#pragma once

class RigidBody : public IRigidBody
{
    btRigidBody *m_pBody = nullptr;
    IGameObject *m_pObject = nullptr;
    
    Vector3 m_position;
    Quaternion m_rotation;

  private:
    void Cleanup();

  public:
    BOOL Initialize(IGameObject* pObj, ICollider *pCollider, float mass, float elasticity, float friction, BOOL useGravity);

    void SetPosition(const Vector3& pos);
    void SetRotation(const Quaternion& q);

    // Inherited via IRigidBody
    void    Update(IGameObject *pObj) override;
    Vector3 GetVelocity() const override;
    void    ApplyImpulseLinear(const Vector3 &impulse) override;
    void    ApplyImpulseAngular(const Vector3 &impulse) override;
    BOOL    IsDynamic() override;

    btRigidBody *Get() const { return m_pBody; }
    IGameObject *GetObj() const { return m_pObject; }
};
