#pragma once

#include "ColliderBase.h"

class GameObject;
class CapsuleCollider : IColliderBase
{
    GameObject *m_pGameObject = nullptr;

    Vector3 m_centerOfMass;
    float m_height;
    float m_radius;

  public:
    BOOL Initialize(GameObject *pObj, Vector3 center, float height, float radius);

    COLLIDER_TYPE GetType() const override { return COLLIDER_TYPE_CAPSULE; }
    Vector3       GetCenter() const override { return m_centerOfMass; }
    Vector3       GetWorldCenter() const override;
    Bounds        GetBounds() const override;
    Bounds        GetWorldBounds() const override;
    Matrix        InertiaTensor() const override;
    BOOL          Intersect(ICollider *pOther) const override;
    BOOL          IntersectRay(const Ray &ray, float *hitt0, float *hitt1) const override;
    BOOL          Intersect(const Bounds &b) const override;
};
