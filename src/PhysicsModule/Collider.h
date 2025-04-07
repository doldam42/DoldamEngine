#pragma once

struct Collider : public ICollider
{
    btCollisionShape *m_pShape = nullptr;
    btCollisionShape *Get() const { return m_pShape; }

    static Collider *CreateBoxCollider(const Vector3 &halfExtents);
    static Collider *CreateSphereCollider(const float radius);
    static Collider *CreateCapsuleCollider(const float radius, const float height);

    ~Collider()
    {
        if (m_pShape)
        {
            delete m_pShape;
            m_pShape = nullptr;
        }
    }

    // Inherited via ICollider
    COLLIDER_TYPE GetType() const override;
    void          Update() override;
    Vector3       GetCenter() const override;
    Vector3       GetWorldCenter() const override;
    Bounds        GetBounds() const override;
    Bounds        GetWorldBounds() const override;
    Matrix        InertiaTensor() const override;
    BOOL          Intersect(ICollider *pOther) const override;
    BOOL          IntersectRay(const Ray &ray, float *hitt0, float *hitt1) const override;
    BOOL          Intersect(const Bounds &b) const override;
    Vector3       Support(const Vector3 dir, const Vector3 pos, const Quaternion orient, const float bias) override;
    float         FastestLinearSpeed(const Vector3 angularVelocity, const Vector3 dir) const override;
};