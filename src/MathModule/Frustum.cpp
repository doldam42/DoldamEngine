#include "Frustum.h"
#include "pch.h"

BoundCheckResult Frustum::CheckBound(const Vector3 &InPoint) const
{
    for (const auto &p : Planes)
    {
        if (p.IsOutside(InPoint))
        {
            return BoundCheckResult::Outside;
        }
        else if (fabs(p.Distance(InPoint) - 0.0f) < 1e-4f)
        {
            return BoundCheckResult::Intersect;
        }
    }
    return BoundCheckResult::Inside;
}

BoundCheckResult Frustum::CheckBound(const Bounds &InBox) const
{
    for (const auto &p : Planes)
    {
        Vector3 pPoint = InBox.mins, nPoint = InBox.maxs;
        if (p.normal.x >= 0.f)
        {
            pPoint.x = InBox.maxs.x;
            nPoint.x = InBox.mins.x;
        }
        if (p.normal.y >= 0.f)
        {
            pPoint.y = InBox.maxs.y;
            nPoint.y = InBox.mins.y;
        }
        if (p.normal.z >= 0.f)
        {
            pPoint.z = InBox.maxs.z;
            nPoint.z = InBox.mins.z;
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

bool Frustum::IsIntersect(const Bounds &InBox) const
{
    for (const auto &p : Planes)
    {
        Vector3 pVertex = InBox.mins, nVertex = InBox.maxs;
        if (p.normal.x >= 0.f)
        {
            pVertex.x = InBox.maxs.x;
            nVertex.x = InBox.mins.x;
        }
        if (p.normal.y >= 0.f)
        {
            pVertex.y = InBox.maxs.y;
            nVertex.y = InBox.mins.y;
        }
        if (p.normal.z >= 0.f)
        {
            pVertex.z = InBox.maxs.z;
            nVertex.z = InBox.mins.z;
        }

        if (p.Distance(nVertex) <= 0.f && p.Distance(pVertex) >= 0.f)
        {
            return true;
        }
    }

    return false;
}
