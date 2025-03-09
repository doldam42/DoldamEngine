#include "pch.h"

#include "GameObject.h"

#include "EllipseCollider.h"

BOOL EllipseCollider::Initialize(GameObject *pObj, Vector3 center, float majorRadius, float minorRadius)
{
    m_pGameObject = pObj;
    m_centerOfMass = center;
    m_majorRadius = majorRadius;
    m_minorRadius = minorRadius;
    return TRUE;
}

Vector3 EllipseCollider::GetWorldCenter() const
{
    Vector3 pos = m_pGameObject->GetPosition();
    return pos + m_centerOfMass;
}

Bounds EllipseCollider::GetBounds() const
{
    Bounds tmp;
    tmp.mins = Vector3(-m_majorRadius);
    tmp.maxs = Vector3(m_majorRadius);

    return tmp;
}

Bounds EllipseCollider::GetWorldBounds() const
{
    Vector3 pos = m_pGameObject->GetPosition();
    Bounds  tmp;
    tmp.mins = pos + Vector3(-m_majorRadius);
    tmp.maxs = pos + Vector3(m_majorRadius);

    return tmp;
}

Matrix EllipseCollider::InertiaTensor() const { return Matrix(); }

BOOL EllipseCollider::Intersect(ICollider *pOther) const { return FALSE; }

BOOL EllipseCollider::Intersect(const Ray &ray, float *hitt0, float *hitt1) const { return 0; }

BOOL EllipseCollider::Intersect(const Bounds &b) const { return 0; }
