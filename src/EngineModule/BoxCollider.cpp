#include "pch.h"

#include "GameObject.h"

#include "BoxCollider.h"
#include "SphereCollider.h"

BOOL BoxCollider::Initialize(GameObject *pObj, const Vector3 center, const Vector3 extents)
{
    m_pGameObject = pObj;

    m_bounds.mins = center - extents;
    m_bounds.maxs = center + extents;

    return TRUE;
}

Bounds BoxCollider::GetWorldBounds() const
{
    const Vector3    pos = m_pGameObject->GetPosition();
    const Quaternion q = m_pGameObject->GetRotation();

    Bounds box;
    m_bounds.Transform(&box, pos, q);
    return box;
}

Vector3 BoxCollider::GetWorldCenter() const
{
    const Vector3 pos = m_pGameObject->GetPosition();
    return pos + m_bounds.Center();
}

Matrix BoxCollider::InertiaTensor() const
{
    constexpr float OneDiv12 = 1 / 12.0f;
    // Inertia tensor for box centered around zero
    const float dx = m_bounds.WidthX();
    const float dy = m_bounds.WidthY();
    const float dz = m_bounds.WidthZ();

    Matrix tensor;
    tensor._11 = (dy * dy + dz * dz) * OneDiv12;
    tensor._22 = (dx * dx + dz * dz) * OneDiv12;
    tensor._33 = (dx * dx + dy * dy) * OneDiv12;

    // Now We need to use the parallel axis theorem to get the inertia tensor for a box
    // that is not centered around the origin

    Vector3 cm = m_bounds.Center();

    const Vector3 R = -cm;
    const float   R2 = R.LengthSquared();

    Matrix patTensor(R2 - R.x * R.x, R.x * R.y, R.x * R.z, 0.0f, R.y * R.x, R2 - R.y * R.y, R.y * R.z, 0.0f, R.z * R.x,
                     R.z * R.y, R2 - R.z * R.z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

    tensor += patTensor;
    return tensor;
}

BOOL BoxCollider::Intersect(ICollider *pOther) const
{
    switch (pOther->GetType())
    {
    case COLLIDER_TYPE_SPHERE: {
        const Vector3   posB = pOther->GetWorldCenter();
        SphereCollider *pB = (SphereCollider *)pOther;
        return GetWorldBounds().DoesIntersect(posB, pB->GetRadius());
    }
    case COLLIDER_TYPE_BOX: {
        return GetWorldBounds().DoesIntersect(pOther->GetWorldBounds());
    }
    default:
        return FALSE;
    }
}

BOOL BoxCollider::IntersectRay(const Ray &ray, float *hitt0, float *hitt1) const
{
    float tmin, tmax;
    if (GetWorldBounds().IntersectP(ray, &tmin, &tmax) && tmin < ray.tmax)
    {
        *hitt0 = tmin;
        *hitt1 = tmax;
        return TRUE;
    }
    return FALSE;
}

BOOL BoxCollider::Intersect(const Bounds &b) const { return b.DoesIntersect(GetWorldBounds()); }

Vector3 BoxCollider::Support(const Vector3 dir, const Vector3 pos, const Quaternion orient, const float bias)
{
    const Matrix m = Matrix::CreateFromQuaternion(orient);

    Vector3 corners[CORNER_COUNT];
    m_bounds.GetCorners(corners);

    Vector3 maxPt = Vector3::Transform(corners[0], m) + pos;
    float   maxDist = maxPt.Dot(dir);
    for (size_t i = 1; i < 8; ++i)
    {
        Vector3 pt = Vector3::Transform(corners[i], m) + pos;
        float   dist = pt.Dot(dir);
        if (dist > maxDist)
        {
            maxDist = dist;
            maxPt = pt;
        }
    }

    Vector3 norm = dir;
    norm.Normalize();
    norm *= bias;

    return maxPt + norm;
}

float BoxCollider::FastestLinearSpeed(const Vector3 angularVelocity, const Vector3 dir) const
{
    Vector3 corners[8];
    m_bounds.GetCorners(corners);

    float maxSpeed = 0.0f;
    for (int i = 0; i < CORNER_COUNT; i++)
    {
        Vector3 r = corners[i];

        Vector3 linearVelocity = angularVelocity.Cross(r);
        float   speed = dir.Dot(linearVelocity);

        if (speed > maxSpeed)
        {
            maxSpeed = speed;
        }
    }
    return maxSpeed;
}
