#pragma once

enum BOUND_CHECK_RESULT
{
    BOUND_CHECK_RESULT_OUTSIDE = 0,
    BOUND_CHECK_RESULT_INTERSECT,
    BOUND_CHECK_RESULT_INSIDE
};

struct Frustum
{
    std::array<Plane, 6> m_planes; // Y+, Y-, X+, X-, Z+, Z-순으로 저장

    inline constexpr Frustum(const std::array<Plane, 6> &inPlane) : m_planes(inPlane)
    {
    }
    inline constexpr BOUND_CHECK_RESULT CheckBound(const Vector3 &InPoint) const;
    inline constexpr BOUND_CHECK_RESULT CheckBound(const Sphere &InSphere) const;
    inline constexpr BOUND_CHECK_RESULT CheckBound(const Box &InBox) const;
    inline constexpr bool             IsIntersect(const Box &InBox) const;
};  
