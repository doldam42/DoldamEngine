#include "pch.h"

#include "Collider.h"

#include "RigidBody.h"

void RigidBody::Initialize(Collider *pCollider, float mass, float elasticity, float friction, BOOL useGravity,
                           BOOL isKinematic)
{
    pCollider->pBody = this;
    m_pCollider = pCollider;
    
    m_invMass = (mass < 1e-4f) ? 0.0f : 1.0f / mass;
    m_elasticity = elasticity;
    m_friction = friction;

    m_useGravity = useGravity;
    m_isKinematic = isKinematic;

    m_linkInPhysics.pItem = this;
    m_linkInPhysics.pPrev = nullptr;
    m_linkInPhysics.pNext = nullptr;
}

void RigidBody::ApplyGravityImpulse(float dt) 
{
    if (!m_useGravity || m_invMass == 0 || m_onGround)
        return;

    const Vector3 gravity(0.0f, -9.8f, 0.0f);
    const float   mass = 1.0f / m_invMass;
    const Vector3 impulseGravity = gravity * mass * dt;
    
    ApplyImpulseLinear(impulseGravity);
}

Matrix RigidBody::GetInverseInertiaTensorWorldSpace() const
{
    Matrix inertiaTensor = m_pCollider->InertiaTensor();
    Matrix invInertiaTensor = inertiaTensor.Invert() * m_invMass;
    Matrix orient = Matrix::CreateFromQuaternion(m_pCollider->Rotation);
    invInertiaTensor = orient * invInertiaTensor * orient.Transpose();
    invInertiaTensor.m[3][3] = 1.0f;
    return invInertiaTensor;
}

Vector3 RigidBody::GetCenterOfMassWorldSpace() const { return m_pCollider->GetPosition(); }

void RigidBody::Update(IGameObject *pObj) 
{ 
    Vector3 pos = m_pCollider->GetPosition();
    Quaternion orient = m_pCollider->GetRotation();
    pObj->SetPosition(pos.x, pos.y, pos.z);
    pObj->SetRotation(orient);
}

void RigidBody::Update(float dt) 
{
    m_pCollider->AddPosition(m_linearVelocity * dt);

    if (m_freezeRotation)
        return;

    Vector3 pos = m_pCollider->GetPosition();
    Vector3 positionCM = GetCenterOfMassWorldSpace();
    Vector3 cmToPos = pos - positionCM;

    // Total Torque is equal to extenal applied torque + internal torque
    // T = T_external + omega x I * omega
    // T = Ia = w x I * w
    // a = I^-1 (w x I * w)
    Matrix orient = Matrix::CreateFromQuaternion(m_pCollider->GetRotation());
    Matrix inertiaTensor = orient * m_pCollider->InertiaTensor() * orient.Transpose();
    Matrix invInertiaTensor = inertiaTensor.Invert();

    const Vector3 Iw = Vector3::Transform(m_angularVelocity, inertiaTensor);

    Vector3 alpha = Vector3::Transform(m_angularVelocity.Cross(Iw), invInertiaTensor);
    m_angularVelocity += alpha * dt;

    // Update Orientation
    Vector3     dAngle = m_angularVelocity * dt;
    const float angleRadians = dAngle.Length();

    Quaternion dq = (angleRadians == 0) ? Quaternion::Identity : Quaternion::CreateFromAxisAngle(dAngle, angleRadians);
    Quaternion o = Quaternion::Concatenate(m_pCollider->GetRotation(), dq);
    o.Normalize();
    m_pCollider->SetRotation(o);

    Vector3 newPos = positionCM + Vector3::Transform(cmToPos, dq);
    m_pCollider->SetPosition(newPos);
}

void RigidBody::ApplyImpulse(const Vector3& impulsePoint, const Vector3& impulse)
{
    if (m_invMass == 0.0f)
        return;

    ApplyImpulseLinear(impulse);

    if (m_freezeRotation)
        return;

    Vector3 position = GetCenterOfMassWorldSpace();
    Vector3 r = impulsePoint - position;
    Vector3 dL = r.Cross(impulse);
    ApplyImpulseAngular(dL);
}

void RigidBody::ApplyImpulseLinear(const Vector3 &impulse) 
{
    if (m_invMass == 0)
        return;

    m_linearVelocity += impulse * m_invMass;
}

void RigidBody::ApplyImpulseAngular(const Vector3 &impulse)
{
    if (m_invMass == 0.0f || m_freezeRotation)
        return;

    // L = I w = r x p
    // dL = I dw = r x J
    // => dw = I ^ -1 * ( r x J )
    m_angularVelocity += Vector3::Transform(impulse, GetInverseInertiaTensorWorldSpace());

    const float maxAngularSpeed = 30.f;
    if (m_angularVelocity.LengthSquared() > maxAngularSpeed * maxAngularSpeed)
    {
        m_angularVelocity.Normalize();
        m_angularVelocity *= maxAngularSpeed;
    }
}

Matrix RigidBody::GetInverseInertiaTensorWorldSpace() const
{
    Matrix inertiaTensor = m_pCollider->InertiaTensor();
    Matrix invInertiaTensor = inertiaTensor.Invert() * m_invMass;
    Matrix orient = Matrix::CreateFromQuaternion(m_pCollider->GetRotation());
    invInertiaTensor = orient * invInertiaTensor * orient.Transpose();
    invInertiaTensor.m[3][3] = 1.0f;
    return invInertiaTensor;
}

Matrix RigidBody::GetInverseInertiaTensorLocalSpace() const
{
    Matrix inertiaTensor = m_pCollider->InertiaTensor();
    Matrix invInertiaTensor = inertiaTensor.Invert() * m_invMass;
    return invInertiaTensor;
}

void RigidBody::Reset() 
{ 
    m_linearVelocity = Vector3::Zero;
    m_angularVelocity = Vector3::Zero;
}
