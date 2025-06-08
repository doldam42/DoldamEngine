#pragma once

#include "ColliderBase.h"

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

class ConvexCollider : Collider
{
    std::vector<Vector3> m_points;
    std::vector<tri_t>   m_triangles;

    Bounds m_bounds;

  public:
    COLLIDER_TYPE GetType() const override { return COLLIDER_TYPE_SPHERE; }

    Bounds GetBounds() const override
    { 
        Bounds b;
        m_bounds.Transform(&b, Matrix::CreateFromQuaternion(Rotation) * Matrix::CreateTranslation(Position));
        return b;
    }

    BOOL RayTest(const Vector3 rayStart, const Vector3 &rayDir, Vector3 *pOutNormal, float *tHit) override;

    Vector3 Support(const Vector3 dir, const Vector3 pos, const Quaternion orient, const float bias) const override;

    ConvexCollider(const Vector3 *points, const int numPoints);
};
