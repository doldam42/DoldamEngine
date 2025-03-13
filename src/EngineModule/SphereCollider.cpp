#include "pch.h"

#include "GameObject.h"

#include "BoxCollider.h"
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
    Bounds tmp;
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

Vector3 SphereCollider::GetWorldCenter() const
{
    const Vector3 pos = m_pGameObject->GetPosition();
    return pos + m_centerOfMass;
}

BOOL SphereCollider::Intersect(ICollider *pOther) const
{
    const Vector3 posA = GetWorldCenter();
    const Vector3 posB = pOther->GetWorldCenter();

    switch (pOther->GetType())
    {
    case COLLIDER_TYPE_SPHERE: {
        Vector3         dummyContactA, dummyContactB;
        SphereCollider *pB = (SphereCollider *)pOther;
        return SphereSphereStatic(m_radius, pB->GetRadius(), posA, posB, &dummyContactA, &dummyContactB);
    }
    case COLLIDER_TYPE_BOX: {
        BoxCollider *pB = (BoxCollider *)pOther;
        return pOther->GetWorldBounds().DoesIntersect(posA, m_radius);
    }
    default:
        return FALSE;
    }
}

BOOL SphereCollider::IntersectRay(const Ray &ray, float *hitt0, float *hitt1) const
{
    float tmin, tmax;
    if (RaySphere(ray.position, ray.direction, GetWorldCenter(), m_radius, &tmin, &tmax) && tmin < ray.tmax)
    {
        *hitt0 = tmin;
        *hitt1 = tmax;
        return TRUE;
    }
    return FALSE;
}

BOOL SphereCollider::Intersect(const Bounds &b) const { return b.DoesIntersect(GetWorldCenter(), m_radius); }
