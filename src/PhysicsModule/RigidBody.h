#pragma once

class RigidBody : public IRigidBody, public btRigidBody
{
    Vector3 m_position;
    Quaternion m_rotation;

  public:
    void SetPosition(const Vector3& pos) override;
    void SetRotation(const Quaternion& q) override;

    void SetPositionInternal(const Vector3 &pos);
    void SetRotationInternal(const Quaternion &q);

    // Inherited via IRigidBody
    void    Update(IGameObject *pObj) override;
    Vector3 GetVelocity() const override;
    void    ApplyImpulseLinear(const Vector3 &impulse) override;
    void    ApplyImpulseAngular(const Vector3 &impulse) override;
    BOOL    IsDynamic() override;

    void  SetUserPtr(void *ptr) override { setUserPointer(ptr); }
    void *GetUserPtr() override { return getUserPointer(); }

    void SetActive(BOOL isActive) override;

    void Reset() override;

    RigidBody(const btRigidBodyConstructionInfo &constructionInfo) : btRigidBody(constructionInfo){};
    ~RigidBody();
};
