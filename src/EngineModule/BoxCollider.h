#pragma once

class GameObject;
class BoxCollider : public ICollider
{
    GameObject *m_pGameObject = nullptr;
    Bounds      m_bounds;

  public:
    BOOL Initialize(GameObject *pObj, const Vector3 &center, const Vector3 &extent);

    // Inherited via ICollider
    COLLIDER_TYPE GetType() const override { return COLLIDER_TYPE_BOX; }
    Vector3       GetCenter() const override { return m_bounds.Center(); }
    Bounds        GetBounds() const override { return m_bounds; }
    Bounds        GetWorldBounds() const override;
    Vector3       GetWorldCenter() const override;

    Matrix        InertiaTensor() const override;

    BOOL Intersect(ICollider *pOther) const override;
    BOOL Intersect(const Ray &ray, float *hitt0, float *hitt1) const override;
    BOOL Intersect(const Bounds &b) const override;

    BoxCollider() = default;
    ~BoxCollider(){};
};
