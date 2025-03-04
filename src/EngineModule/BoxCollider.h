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
    Bounds        GetBounds() const override;
    Bounds        GetWorldBounds() const override;

    Matrix        InertiaTensor() const override;

    BoxCollider() = default;
    ~BoxCollider(){};
};
