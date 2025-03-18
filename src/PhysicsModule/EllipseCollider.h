#pragma once

class IGameObject;

// EllipseCollider는 Y축이 장축이고 X,Z가 단축인 타원이다.
class EllipseCollider : public ICollider
{
    IGameObject *m_pIGameObject = nullptr;

    Vector3 m_centerOfMass;
    Vector3 m_worldCenterOfMass;

    float m_majorRadius; // 장축
    float m_minorRadius; // 단축

  public:
    BOOL Initialize(IGameObject *pObj, Vector3 center, float majorRadius, float minorRadius);

    // Inherited via ICollider
    DP_COLLIDER_TYPE GetType() const override { return DP_COLLIDER_TYPE_ELLIPSE; }
    Vector3       GetCenter() const override { return m_centerOfMass; }
    Vector3       GetWorldCenter() const override { return m_worldCenterOfMass; }
    Bounds        GetBounds() const override;
    Bounds        GetWorldBounds() const override;
    Matrix        InertiaTensor() const override;
    BOOL          Intersect(ICollider *pOther) const override;
    BOOL          IntersectRay(const Ray &ray, float *hitt0, float *hitt1) const override;
    BOOL          Intersect(const Bounds &b) const override;

    void Update() override;
};
