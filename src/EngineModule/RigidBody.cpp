#include "pch.h"

#include "GameObject.h"

#include "RigidBody.h"

void RigidBody::Cleanup()
{
}

void RigidBody::Initialize(GameObject *pObj, ICollider *pCollider, float mass, float elasticity, float friction,
                           BOOL useGravity, BOOL isKinematic)
{
    m_pCollider = pCollider;

    m_invMass = (mass < 1e-4f) ? 0.0f : 1.0f / mass;
    m_elasticity = elasticity;
    m_friction = friction;

    m_pGameObject = pObj;

    m_useGravity = useGravity;
    m_isKinematic = isKinematic;
}

void RigidBody::ApplyGravityImpulse(const float dt)
{
    if (!m_useGravity || m_invMass == 0)
        return;

    const Vector3 gravity(0.0f, -9.8f, 0.0f);
    const float   mass = 1.0f / m_invMass;
    const Vector3 impulseGravity = gravity * mass * dt;

    ApplyImpulseLinear(impulseGravity);
}

void RigidBody::ApplyImpulse(const Vector3 &impulsePoint, const Vector3 &impulse)
{
    if (m_invMass == 0.0f)
        return;

    ApplyImpulseLinear(impulse);

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
    if (m_invMass == 0.0f)
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

void RigidBody::Update(float dt)
{
    Vector3 deltaPos(m_linearVelocity * dt);
    m_pGameObject->AddPosition(&deltaPos);

    Vector3 pos = GetPosition();
    Vector3 positionCM = GetCenterOfMassWorldSpace();
    Vector3 cmToPos = pos - positionCM;

    // Total Torque is equal to extenal applied torque + internal torque
    // T = T_external + omega x I * omega
    // T = Ia = w x I * w
    // a = I^-1 (w x I * w)
    Matrix orient = Matrix::CreateFromQuaternion(GetOrient());
    Matrix inertiaTensor = orient * m_pCollider->InertiaTensor() * orient.Transpose();
    Matrix invInertiaTensor = inertiaTensor.Invert();

    const Vector3 Iw = Vector3::Transform(m_angularVelocity, inertiaTensor);

    Vector3 alpha = Vector3::Transform(m_angularVelocity.Cross(Iw), invInertiaTensor);
    m_angularVelocity += alpha * dt;

    // Update Orientation
    Vector3     dAngle = m_angularVelocity * dt;
    const float angleRadians = dAngle.Length();

    Quaternion dq = (angleRadians == 0) ? Quaternion::Identity : Quaternion::CreateFromAxisAngle(dAngle, angleRadians);
    Quaternion o = Quaternion::Concatenate(GetOrient(), dq);
    o.Normalize();
    m_pGameObject->SetRotation(&o);

    Vector3 newPos = positionCM + Vector3::Transform(cmToPos, dq);
    m_pGameObject->SetPosition(newPos.x, newPos.y, newPos.z);
}

Vector3 RigidBody::GetPosition() const { return m_pGameObject->GetPosition(); }

Quaternion RigidBody::GetOrient() const { return m_pGameObject->GetRotation(); }

void RigidBody::AddPosition(const Vector3 &deltaPos) { m_pGameObject->AddPosition(&deltaPos); }

Vector3 RigidBody::GetCenterOfMassWorldSpace() const
{
    const Vector3 centerOfMass = m_pCollider->GetCenter();
    const Vector3 pos = m_pGameObject->GetPosition() + Vector3::Transform(centerOfMass, m_pGameObject->GetRotation());
    return pos;
}

Vector3 RigidBody::GetCenterOfMassLocalSpace() const { return m_pCollider->GetCenter(); }

Vector3 RigidBody::WorldSpaceToLocalSpace(const Vector3 &point) const
{
    Vector3    tmp = point - GetCenterOfMassWorldSpace();
    Quaternion orient = m_pGameObject->GetRotation();

    Quaternion inverseOrient;
    orient.Inverse(inverseOrient);
    Vector3 localSpace = Vector3::Transform(tmp, inverseOrient);
    return localSpace;
}

Vector3 RigidBody::LocalSpaceToWorldSpace(const Vector3 &point) const
{
    Vector3 worldSpace = GetCenterOfMassWorldSpace() + Vector3::Transform(point, m_pGameObject->GetRotation());
    return worldSpace;
}

Matrix RigidBody::GetInverseInertiaTensorWorldSpace() const
{
    Matrix inertiaTensor = m_pCollider->InertiaTensor();
    Matrix invInertiaTensor = inertiaTensor.Invert() * m_invMass;
    Matrix orient = Matrix::CreateFromQuaternion(GetOrient());
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

Bounds RigidBody::GetBounds() const
{
    return m_pCollider->GetWorldBounds();
}

RigidBody::~RigidBody() { Cleanup(); }
