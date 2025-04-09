#pragma once

#include "ColliderBase.h"
class GameObject;
class BoxCollider : public IColliderBase
{
  public:
    static constexpr size_t CORNER_COUNT = 8;

  private:
    GameObject *m_pGameObject = nullptr;

    Matrix m_inertiaTensor;
    Bounds m_bounds;
    Bounds m_worldBounds;

    void InitTensor();

  public:
    BOOL Initialize(GameObject *pObj, const Vector3 center, const Vector3 halfExtents);

    void Update();

    // Inherited via ICollider
    COLLIDER_TYPE GetType() const override { return COLLIDER_TYPE_BOX; }
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
