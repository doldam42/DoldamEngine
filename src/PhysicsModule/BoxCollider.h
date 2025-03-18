#pragma once

class BoxCollider : public ICollider
{
  public:
    static constexpr size_t CORNER_COUNT = 8;

  private:
    IGameObject *m_pIGameObject = nullptr;

    Matrix m_inertiaTensor;
    Bounds m_bounds;
    Bounds m_worldBounds;

    void InitTensor();

  public:
    BOOL Initialize(IGameObject *pObj, const Vector3 center, const Vector3 extents);

    void Update() override;

    // Inherited via ICollider
    DP_COLLIDER_TYPE GetType() const override { return DP_COLLIDER_TYPE_BOX; }
    Vector3       GetCenter() const override { return m_bounds.Center(); }
    Bounds        GetBounds() const override { return m_bounds; }
    Bounds        GetWorldBounds() const override { return m_worldBounds; }
    Vector3       GetWorldCenter() const override { return m_worldBounds.Center(); }

    Matrix InertiaTensor() const override { return m_inertiaTensor; }

    BOOL Intersect(ICollider *pOther) const override;
    BOOL IntersectRay(const Ray &ray, float *hitt0, float *hitt1) const override;
    BOOL Intersect(const Bounds &b) const override;

    Vector3 Support(const Vector3 dir, const Vector3 pos, const Quaternion orient, const float bias) override;
    float   FastestLinearSpeed(const Vector3 angularVelocity, const Vector3 dir) const override;

    BoxCollider() = default;
    ~BoxCollider(){};
};
