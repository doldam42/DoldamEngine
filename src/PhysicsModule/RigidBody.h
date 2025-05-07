#pragma once
class RigidBody : public IRigidBody
{
  public:
    // Inherited via IRigidBody
    void Update(IGameObject *pObj) override;
    Vector3 GetVelocity() const override;
    void    ApplyImpulseLinear(const Vector3 &impulse) override;
    void    ApplyImpulseAngular(const Vector3 &impulse) override;
    void    SetActive(BOOL isActive) override;
    void    Reset() override;
};
