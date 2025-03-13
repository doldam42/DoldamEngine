#pragma once

struct tri_t
{
    int a;
    int b;
    int c;
};

struct edge_t
{
    int a;
    int b;

    bool operator==(const edge_t &rhs) const { return ((a == rhs.a && b == rhs.b) || (a == rhs.b && b == rhs.a)); }
};

class GameObject;
class ConvexCollider : public ICollider
{
    GameObject *m_pGameObject = nullptr;

    std::vector<Vector3> m_points;
    std::vector<tri_t>   m_triangles;

    Vector3 m_centerOfMass;
    Bounds m_bounds;
    Matrix m_inertiaTensor;

  public:
    BOOL          Initialize(GameObject *pObj, const Vector3 *points, const int num);
    // Inherited via ICollider
    COLLIDER_TYPE GetType() const override { return COLLIDER_TYPE_CONVEX; }
    Vector3       GetCenter() const override { return m_centerOfMass; }
    Vector3       GetWorldCenter() const override;
    Bounds        GetBounds() const override { return m_bounds; }
    Bounds        GetWorldBounds() const override;
    Matrix        InertiaTensor() const override;
    BOOL          Intersect(ICollider *pOther) const override;
    BOOL          IntersectRay(const Ray &ray, float *hitt0, float *hitt1) const override;
    BOOL          Intersect(const Bounds &b) const override;
    Vector3       Support(const Vector3 dir, const Vector3 pos, const Quaternion orient, const float bias) override;
    float         FastestLinearSpeed(const Vector3 angularVelocity, const Vector3 dir) const override;
};
