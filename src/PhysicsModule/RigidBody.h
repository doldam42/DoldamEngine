#pragma once

struct Collider;
class RigidBody : public IRigidBody
{
    Collider *m_pCollider = nullptr;
    
    Vector3 m_linearVelocity = Vector3::Zero;
    Vector3 m_angularVelocity = Vector3::Zero;

    float m_invMass = 0.0f;
    float m_elasticity = 1.0f;
    float m_friction = 0.0f;

    BOOL m_isActive = TRUE;
    BOOL m_isKinematic = TRUE;
    BOOL m_useGravity = TRUE;
    BOOL m_onGround = FALSE;
    BOOL m_freezeRotation = FALSE;

    Matrix GetInverseInertiaTensorWorldSpace() const;
    Vector3 GetCenterOfMassWorldSpace() const;
    
  public:
    void Initialize(Collider *pCollider, float mass, float elasticity, float friction, BOOL useGravity = TRUE,
                    BOOL isKinematic = FALSE);

    void Update(IGameObject *pObj) override;
    Vector3 GetVelocity() const override { return m_linearVelocity; }
    void    ApplyImpulseLinear(const Vector3 &impulse) override;
    void    ApplyImpulseAngular(const Vector3 &impulse) override;
    void    SetActive(BOOL isActive) override { m_isActive = isActive; }
    void    Reset() override;

    void Update(float dt);
};
