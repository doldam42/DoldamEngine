#pragma once

class GameObject;
class PhysicsComponent : public IPhysicsComponent
{
  public:
    Vector3 m_linearVelocity = Vector3::Zero;

    float m_invMass = 0.0f;
    float m_elasticity = 1.0f;

    Shape *m_pShape = nullptr;
    GameObject *m_pGameObject = nullptr;

  private:
    void Cleanup();

  public:
    // Inherited via IPhysicsComponent
    void    Initialize(GameObject* pObj, const Shape *pInShape, float mass, float elasticity);

    Vector3 GetVelocity() const override { return m_linearVelocity; }
    void    ApplyImpulseLinear(const Vector3 &impulse) override;

    void Update(float dt);

    Vector3 GetCenterOfMassWorldSpace() const;
    Vector3 GetCenterOfMassLocalSpace() const;

    Vector3 WorldSpaceToLocalSpace(const Vector3 &point) const;
    Vector3 LocalSpaceToWorldSpace(const Vector3 &point) const;

    PhysicsComponent() = default;
    ~PhysicsComponent();
};
