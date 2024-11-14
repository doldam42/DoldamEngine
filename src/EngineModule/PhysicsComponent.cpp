#include "pch.h"

#include "GameObject.h"

#include "PhysicsComponent.h"

void PhysicsComponent::Cleanup()
{
    if (m_pShape)
    {
        delete m_pShape;
        m_pShape = nullptr;
    }
}

void PhysicsComponent::Initialize(GameObject *pObj, const Shape *pInShape, float mass, float elasticity)
{
    if (pInShape->GetType() == SHAPE_TYPE_SPHERE)
    {
        m_pShape = new Sphere(*((Sphere *)pInShape));
    }

    m_invMass = (mass < 1e-4f) ? 0.0f : 1.0f / mass;
    m_elasticity = elasticity;

    m_pGameObject = pObj;
}

void PhysicsComponent::ApplyImpulseLinear(const Vector3 &impulse)
{
    if (m_invMass == 0)
        return;

    m_linearVelocity += impulse * m_invMass;
}

void PhysicsComponent::Update(float dt)
{
    Vector3 deltaPos(m_linearVelocity * dt);
    m_pGameObject->AddPosition(&deltaPos);
}

Vector3 PhysicsComponent::GetCenterOfMassWorldSpace() const
{
    const Vector3 centerOfMass = m_pShape->GetCenterOfMass();
    const Vector3 pos = m_pGameObject->GetPosition() + Vector3::Transform(centerOfMass, m_pGameObject->GetRotation());
    return pos;
}

Vector3 PhysicsComponent::GetCenterOfMassLocalSpace() const { return m_pShape->GetCenterOfMass(); }

Vector3 PhysicsComponent::WorldSpaceToLocalSpace(const Vector3 &point) const
{
    Vector3    tmp = point - GetCenterOfMassWorldSpace();
    Quaternion orient = m_pGameObject->GetRotation();

    Quaternion inverseOrient;
    orient.Inverse(inverseOrient);
    Vector3 localSpace = Vector3::Transform(tmp, inverseOrient);
    return localSpace;
}

Vector3 PhysicsComponent::LocalSpaceToWorldSpace(const Vector3 &point) const
{
    Vector3 worldSpace = GetCenterOfMassWorldSpace() + Vector3::Transform(point, m_pGameObject->GetRotation());
    return worldSpace;
}

PhysicsComponent::~PhysicsComponent() { Cleanup(); }
