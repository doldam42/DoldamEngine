#include "pch.h"

#include "GameObject.h"
#include "Shape.h"

#include "PhysicsComponent.h"

BOOL PhysicsComponent::Initialize(GameObject *pObj, SHAPE_TYPE shapeType, const Shape *pInShape)
{
    m_pGameObject = pObj;
    switch (shapeType)
    {
    case SHAPE_TYPE_BOX:
        const ShapeBox *pInBox = (const ShapeBox *)pInShape;
        m_pShape = new ShapeBox(*pInBox);
        break;
    case SHAPE_TYPE_SPHERE:
        const ShapeSphere *pInSphere = (const ShapeSphere *)pInShape;
        m_pShape = new ShapeSphere(*pInSphere);
        break;
    default:
        break;
    }
}

void PhysicsComponent::Update(const float dt)
{
    m_position = m_pGameObject->GetPosition();
    m_orientation = m_pGameObject->GetRotation();

    m_position += 
}

Vector3 PhysicsComponent::GetCenterOfMassWorldSpace() const
{
    const Vector3 centerOfMass = m_pShape->GetCenterOfMass();

    Matrix m = Matrix::CreateFromQuaternion(m_orientation) * Matrix::CreateTranslation(m_position);

    const Vector3 pos = Vector3::Transform(centerOfMass, m);

    return pos;
}

Vector3 PhysicsComponent::GetCenterOfMassLocalSpace() const
{
    const Vector3 centerOfMass = m_pShape->GetCenterOfMass();
    return centerOfMass;
}

Vector3 PhysicsComponent::WorldToLocal(const Vector3 &worldPoint) const
{
    Vector3    tmp = worldPoint - GetCenterOfMassWorldSpace();
    Quaternion inverseOrient;
    m_orientation.Inverse(inverseOrient);
    Vector3 localSpace = Vector3::Transform(tmp, Matrix::CreateFromQuaternion(inverseOrient));
    return localSpace;
}

Vector3 PhysicsComponent::LocalToWorld(const Vector3 &worldPoint) const
{
    Vector3 tmp;
    worldPoint.Transform(tmp, m_orientation);
    Vector3 worldSpace = GetCenterOfMassWorldSpace() + tmp;
    return worldSpace;
}
