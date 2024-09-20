#pragma once

enum class BoundCheckResult : UINT
{
    Outside = 0,
    Intersect,
    Inside
};

struct Frustum
{
  public:
    Plane Planes[6]; // +Y, -Y, +X, -X, +Z, -Z

  public:
    inline BoundCheckResult CheckBound(const Vector3 &inPoint) const;
    inline BoundCheckResult CheckBound(const Sphere &inSphere) const;
    inline BoundCheckResult CheckBound(const Box &inBox) const;

    Frustum() = default;
    Frustum(const Plane* pInPlanes)
    {
        for (int i = 0; i < 6; i++)
        {
            this->Planes[i] = pInPlanes[i];
        }
    }
};

inline BoundCheckResult Frustum::CheckBound(const Vector3 &inPoint) const
{
    for (const auto &p : Planes)
    {
        if (p.IsOutside(inPoint))
        {
            return BoundCheckResult::Outside;
        }
        else if (p.Distance(inPoint) <= 1e-8f)
        {
            return BoundCheckResult::Intersect;
        }
    }

    return BoundCheckResult::Inside;
}
inline BoundCheckResult Frustum::CheckBound(const Sphere &inSphere) const
{
    for (const auto &p : Planes)
    {
        if (p.Distance(inSphere.Center) > inSphere.Radius)
        {
            return BoundCheckResult::Outside;
        }
        else if (abs(p.Distance(inSphere.Center)) <= inSphere.Radius)
        {
            return BoundCheckResult::Intersect;
        }
    }

    return BoundCheckResult::Inside;
}
inline BoundCheckResult Frustum::CheckBound(const Box &inBox) const
{
    for (const auto &p : Planes)
    {
        Vector3 pPoint = inBox.Min, nPoint = inBox.Max;
        if (p.Normal().x >= 0.f)
        {
            pPoint.x = inBox.Max.x;
            nPoint.x = inBox.Min.x;
        }
        if (p.Normal().y >= 0.f)
        {
            pPoint.y = inBox.Max.y;
            nPoint.y = inBox.Min.y;
        }
        if (p.Normal().z >= 0.f)
        {
            pPoint.z = inBox.Max.z;
            nPoint.z = inBox.Min.z;
        }

        if (p.Distance(nPoint) > 0.f)
        {
            return BoundCheckResult::Outside;
        }
        if (p.Distance(nPoint) <= 0.f && p.Distance(pPoint) >= 0.f)
        {
            return BoundCheckResult::Intersect;
        }
    }

    return BoundCheckResult::Inside;
}