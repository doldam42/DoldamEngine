#pragma once

class GameObject;
class SphereCollider : public ICollider
{
    GameObject *m_pGameObject = nullptr;
    Vector3     m_centerOfMass;
    float       m_radius = 0.0f;

  public:
    BOOL Initialize(GameObject *pObj, const Vector3 &center, const float radius);

    // Inherited via ICollider
    Bounds GetBounds() const override;
    Bounds GetWorldBounds() const override;

    // Getter
    COLLIDER_TYPE GetType() const override { return COLLIDER_TYPE_SPHERE; }
    Vector3       GetCenter() const override { return m_centerOfMass; }
    float         GetRadius() const { return m_radius; }
    Matrix        InertiaTensor() const override
    {
        Matrix tensor = Matrix::Identity;
        tensor.m[0][0] = 2.0f * m_radius * m_radius / 5.0f;
        tensor.m[1][1] = 2.0f * m_radius * m_radius / 5.0f;
        tensor.m[2][2] = 2.0f * m_radius * m_radius / 5.0f;

        return tensor;
    }

    SphereCollider() = default;
    ~SphereCollider() {}
};
