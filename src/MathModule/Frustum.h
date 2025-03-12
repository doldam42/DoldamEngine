#pragma once

#include "Plane.h"

enum class BoundCheckResult : UINT
{
    Outside = 0,
    Intersect,
    Inside
};

struct Frustum
{
    // ������
    Frustum() = default;
    Frustum(const std::array<Plane, 6> &InPlanes) : Planes(InPlanes){}

    // ����Լ�
    BoundCheckResult CheckBound(const Vector3 &InPoint) const;
    BoundCheckResult CheckBound(const Bounds &InBox) const;
    bool             IsIntersect(const Bounds &InBox) const;

    // �������
    std::array<Plane, 6> Planes; // Y+, Y-, X+, X-, Z+, Z- ������ ����
};
