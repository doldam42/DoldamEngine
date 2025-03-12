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
    // 생성자
    Frustum() = default;
    Frustum(const std::array<Plane, 6> &InPlanes) : Planes(InPlanes){}

    // 멤버함수
    BoundCheckResult CheckBound(const Vector3 &InPoint) const;
    BoundCheckResult CheckBound(const Bounds &InBox) const;
    bool             IsIntersect(const Bounds &InBox) const;

    // 멤버변수
    std::array<Plane, 6> Planes; // Y+, Y-, X+, X-, Z+, Z- 순으로 저장
};
