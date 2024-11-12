#pragma once

class GameObject;
class Shape;
class PhysicsManager;
class PhysicsComponent
{
  public:
    Vector3 m_position;
    Quaternion m_orientation;

    Vector3 m_linearVelocity = Vector3::Zero;
    Vector3 m_angularVelocity = Vector3::Zero;

    float m_invMass = 0.0f;
    float m_elasticity = 1.0f;
    float m_friction;

    Shape *m_pShape = nullptr;
    GameObject *m_pGameObject = nullptr;

  public:
    // Physics
    BOOL Initialize(GameObject* pObj, SHAPE_TYPE shapeType, const Shape *pInShape);

    void Update(const float dt);

    void ApplyImpulse(const Vector3 &impulsepoint, const Vector3 &impulse);
    void ApplyImpulseLinear(const Vector3 &impulse);
    void ApplyImpulseAngular(const Vector3 &impulse);

    // Getter
    Vector3 GetCenterOfMassWorldSpace() const;
    Vector3 GetCenterOfMassLocalSpace() const;

    Vector3 WorldToLocal(const Vector3 &worldPoint) const;
    Vector3 LocalToWorld(const Vector3 &worldPoint) const;

    Matrix GetInverseInertiaTensorLocalSpace() const;
    Matrix GetInverseInertiaTensorWorldSpace() const;

    Vector3 GetVelocity() const { return m_linearVelocity; }
};
