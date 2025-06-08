#pragma once

#include "Shape.h"

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

struct ConvexShape : public Shape
{
    std::vector<Vector3> Points;
    std::vector<tri_t>   Triangles;

    SHAPE_TYPE GetType() const override { return SHAPE_TYPE_CONVEX; }

    BOOL RayTest(const Ray &ray, const Vector3 &pos, const Quaternion &orient, Vector3 *pOutNormal,
                 float *tHit) override;

    Vector3 Support(const Vector3 &dir, const Vector3 &pos, const Quaternion &orient, const float bias) const override;

    ConvexShape(const Vector3 *points, const int numPoints);
};
