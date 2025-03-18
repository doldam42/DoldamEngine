#pragma once

class GameObject;
class SphereCollider : public ICollider
{
    GameObject *m_pGameObject = nullptr;
    Matrix      m_inertiaTensor;
    Vector3     m_centerOfMass;
    Vector3     m_worldCenterOfMass;
    float       m_radius = 0.0f;

  public:
    BOOL Initialize(GameObject *pObj, const Vector3 &center, const float radius);

    // Inherited via ICollider
    Bounds GetBounds() const override;
    Bounds GetWorldBounds() const override;

    // Getter
    COLLIDER_TYPE GetType() const override { return COLLIDER_TYPE_SPHERE; }
    Vector3       GetCenter() const override { return m_centerOfMass; }
    Vector3       GetWorldCenter() const override { return m_worldCenterOfMass; }
    float         GetRadius() const { return m_radius; }
    Matrix        InertiaTensor() const override { return m_inertiaTensor; }

    BOOL Intersect(ICollider *pOther) const override;
    BOOL IntersectRay(const Ray &ray, float *hitt0, float *hitt1) const override;
    BOOL Intersect(const Bounds &b) const override;

    Vector3 Support(const Vector3 dir, const Vector3 pos, const Quaternion orient, const float bias) override
    {
        return pos + dir * (m_radius + bias);
    }
    float FastestLinearSpeed(const Vector3 angularVelocity, const Vector3 dir) const override { return 0.0f; }

    void Update() override;

    SphereCollider() = default;
    ~SphereCollider() {}
};
