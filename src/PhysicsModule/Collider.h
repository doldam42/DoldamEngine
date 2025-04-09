#pragma once

struct Collider : public ICollider
{
    btCollisionShape *m_pShape = nullptr;
    btCollisionShape *Get() const { return m_pShape; }

    static Collider *CreateBoxCollider(const Vector3 &halfExtents);
    static Collider *CreateSphereCollider(const float radius);
    static Collider *CreateCapsuleCollider(const float radius, const float height);
    static Collider *CreateConvexCollider(const Vector3 *points, const int numPoints);

    ~Collider()
    {
        if (m_pShape)
        {
            delete m_pShape;
            m_pShape = nullptr;
        }
    }
};