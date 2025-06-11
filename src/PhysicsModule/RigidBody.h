#pragma once

struct Collider;
class RigidBody : public IRigidBody
{
    Collider *m_pCollider = nullptr;

    float m_invMass = 0.0f;
    float m_elasticity = 1.0f;
    float m_friction = 0.0f;

    BOOL m_isActive = TRUE;
    BOOL m_isKinematic = TRUE;
    BOOL m_useGravity = TRUE;
    BOOL m_onGround = FALSE;
    BOOL m_freezeRotation = FALSE;

    Vector3 m_linearVelocity = Vector3::Zero;
    Vector3 m_angularVelocity = Vector3::Zero;

  public:
    SORT_LINK m_linkInPhysics = {};

    void Initialize(Collider *pCollider, float mass, float elasticity, float friction, BOOL useGravity = TRUE,
                    BOOL isKinematic = FALSE);

    void ApplyGravityImpulse(float dt);

    void    Update(IGameObject *pObj) override;
    Vector3 GetVelocity() const override { return m_linearVelocity; }

    void    ApplyImpulse(const Vector3 &impulsePoint, const Vector3 &impulse);
    void    ApplyImpulseLinear(const Vector3 &impulse) override;
    void    ApplyImpulseAngular(const Vector3 &impulse) override;
    void    SetActive(BOOL isActive) override { m_isActive = isActive; }
    void    Reset() override;

    float GetInvMass() const { return m_invMass; }
    float GetElasticity() const { return m_elasticity; }
    float GetFriction() const { return m_friction; }

    const Vector3& GetLinearVelocity() const { return m_linearVelocity; }
    const Vector3& GetAngularVelocity() const { return m_angularVelocity; }

    Vector3 GetCenterOfMassWorldSpace() const;

    Matrix GetInverseInertiaTensorWorldSpace() const;
    Matrix GetInverseInertiaTensorLocalSpace() const;

    void Update(float dt);
};
