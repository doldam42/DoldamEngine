#pragma once

class GameObject;
class PhysicsComponent : public IPhysicsComponent
{
  public:
    Vector3 m_linearVelocity = Vector3::Zero;
    Vector3 m_angularVelocity = Vector3::Zero;

    float m_invMass = 0.0f;
    float m_elasticity = 1.0f;
    float m_friction = 0.0f;

    Shape      *m_pShape = nullptr;
    GameObject *m_pGameObject = nullptr;

  private:
    void Cleanup();

  public:
    // Inherited via IPhysicsComponent
    void Initialize(GameObject *pObj, const Shape *pInShape, float mass, float elasticity, float friction);

    Vector3 GetVelocity() const override { return m_linearVelocity; }
    Bounds  GetBounds() const override;

    void ApplyGravityImpulse(const float dt);

    void ApplyImpulse(const Vector3 &impulsePoint, const Vector3 &impulse);
    void ApplyImpulseLinear(const Vector3 &impulse) override;
    void ApplyImpulseAngular(const Vector3 &impulse) override;

    void Update(float dt);

    Vector3    GetPosition() const;
    Quaternion GetOrient() const;

    void AddPosition(const Vector3 &deltaPos);

    Vector3 GetCenterOfMassWorldSpace() const;
    Vector3 GetCenterOfMassLocalSpace() const;

    Vector3 WorldSpaceToLocalSpace(const Vector3 &point) const;
    Vector3 LocalSpaceToWorldSpace(const Vector3 &point) const;

    Matrix GetInverseInertiaTensorWorldSpace() const;
    Matrix GetInverseInertiaTensorLocalSpace() const;

    PhysicsComponent() = default;
    ~PhysicsComponent();
};
