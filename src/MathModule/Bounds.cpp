#include "pch.h"
#include "Bounds.h"

const Bounds &Bounds::operator=(const Bounds &rhs)
{
    Min = rhs.Min;
    Max = rhs.Max;
}

bool Bounds::DoesIntersect(const Bounds &rhs) const
{
    if (Max.x < rhs.Min.x || Max.y < rhs.Min.y || Max.z < rhs.Min.z)
    {
        return false;
    }
    if (rhs.Max.x < Min.x || rhs.Max.y < Min.y || rhs.Max.z < Min.z)
    {
        return false;
    }
    return true;
}

void Bounds::Expand(const Vector3 *points, const UINT numPoints) 
{ 
    for (UINT i = 0; i < numPoints; i++)
    {
        Expand(points[i]);
    }
}

void Bounds::Expand(const Vector3 &rhs) 
{
    if (rhs.x < Min.x)
    {
        Min.x = rhs.x;
    }
    if (rhs.y < Min.y)
    {
        Min.y = rhs.y;
    }
    if (rhs.z < Min.z)
    {
        Min.z = rhs.z;
    }

    if (rhs.x > Max.x)
    {
        Max.x = rhs.x;
    }
    if (rhs.y > Max.y)
    {
        Max.y = rhs.y;
    }
    if (rhs.z > Max.z)
    {
        Max.z = rhs.z;
    }
}

void Bounds::Expand(const Bounds &rhs) 
{
    Expand(rhs.Min);
    Expand(rhs.Max);
}
