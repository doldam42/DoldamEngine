#include "pch.h"

#include "GameObject.h"

#include "SphereCollider.h"

BOOL SphereCollider::Initialize(GameObject *pObj, const Vector3 &center, const float radius) 
{
    m_pGameObject = pObj;
    m_centerOfMass = center;
    m_radius = radius;

    return TRUE;
}

Bounds SphereCollider::GetBounds() const 
{ 
    Bounds  tmp;
    tmp.mins = Vector3(-m_radius);
    tmp.maxs = Vector3(m_radius);
    return tmp;
}

Bounds SphereCollider::GetWorldBounds() const
{
    Vector3 pos = m_pGameObject->GetPosition();
    Bounds  tmp;
    tmp.mins = Vector3(-m_radius) + pos;
    tmp.maxs = Vector3(m_radius) + pos;
    return tmp;
}
