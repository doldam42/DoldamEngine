#include "pch.h"

#include "GameObject.h"

#include "CapsuleCollider.h"

BOOL CapsuleCollider::Initialize(GameObject *pObj, Vector3 center, float height, float radius)
{
    m_pGameObject = pObj;

    m_centerOfMass = center;
    m_height = height;
    m_radius = radius;

    return TRUE;
}

Vector3 CapsuleCollider::GetWorldCenter() const
{
    const Vector3 pos = m_pGameObject->GetPosition();
    return pos + GetCenter();
}

Bounds CapsuleCollider::GetBounds() const
{
    Bounds b;

    Vector3 tmp(m_radius + m_height * 0.5f);
    b.mins = -tmp;
    b.maxs = tmp;
    
    return b;
}

Bounds CapsuleCollider::GetWorldBounds() const
{
    Bounds  b;
    Vector3 pos = m_pGameObject->GetPosition();

    Vector3 tmp(m_radius + m_height * 0.5f);

    b.mins = pos - tmp;
    b.maxs = pos + tmp;

    return b;
}

Matrix CapsuleCollider::InertiaTensor() const
{
    constexpr float oneDiv3 = 1.0f / 3.0f;
    constexpr float oneDiv8 = 1.0f / 8.0f;
    constexpr float oneDiv12 = 1.0f / 12.0f;
    constexpr float density = 1.0f;
    
    Matrix inertia = Matrix::Identity;
    float cM;  // cylinder mass
    float hsM; // mass of hemispheres
    float rSq = m_radius * m_radius;
    cM = XM_PI * m_height * rSq * density;
    hsM = XM_2PI * oneDiv3 * rSq * m_radius * density; // from cylinder
    inertia._22 = rSq * cM * 0.5f;
    inertia._11 = inertia._33 = inertia._22 * 0.5f + cM * m_height * m_height * oneDiv12; // from hemispheres
    float temp0 = hsM * 2.0f * rSq / 5.0f;
    inertia._22 += temp0 * 2.0f;
    float temp1 = m_height * 0.5f;
    float temp2 = temp0 + hsM * (temp1 * temp1 + 3.0f * oneDiv8 * m_height * m_radius);
    inertia._11 += temp2 * 2.0f;
    inertia._33 += temp2 * 2.0f;
    inertia._12 = inertia._13 = inertia._21 = inertia._23 = inertia._31 = inertia._32 = 0.0f;

    return inertia;
}

BOOL CapsuleCollider::Intersect(ICollider *pOther) const { return 0; }

BOOL CapsuleCollider::IntersectRay(const Ray &ray, float *hitt0, float *hitt1) const { return 0; }

BOOL CapsuleCollider::Intersect(const Bounds &b) const { return 0; }
