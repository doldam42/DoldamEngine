#include "pch.h"

#include "EllipseCollider.h"

BOOL EllipseCollider::Initialize(IGameObject *pObj, Vector3 center, float majorRadius, float minorRadius)
{
    m_pIGameObject = pObj;
    m_centerOfMass = center;
    m_majorRadius = majorRadius;
    m_minorRadius = minorRadius;

    Update();
    return TRUE;
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
    Vector3 pos = m_pIGameObject->GetPosition();
    Bounds  tmp;
    tmp.mins = pos + Vector3(-m_majorRadius);
    tmp.maxs = pos + Vector3(m_majorRadius);

    return tmp;
}

Matrix EllipseCollider::InertiaTensor() const { return Matrix(); }

BOOL EllipseCollider::Intersect(ICollider *pOther) const { return FALSE; }

BOOL EllipseCollider::IntersectRay(const Ray &ray, float *hitt0, float *hitt1) const
{
    return RayEllipse(ray.position, ray.direction, GetCenter(), m_majorRadius, m_minorRadius, hitt0, hitt1);
}

BOOL EllipseCollider::Intersect(const Bounds &b) const { return 0; }

void EllipseCollider::Update()
{
    Vector3 pos = m_pIGameObject->GetPosition();
    m_worldCenterOfMass = pos + m_centerOfMass;
}
