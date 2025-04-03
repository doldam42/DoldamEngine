#include "pch.h"

#include "MathUtils.h"

Bounds GetBoundingBox(const BasicVertex *pVertice, UINT numVertice)
{
    if (numVertice == 0)
        return Bounds();

    Vector3 minCorner = pVertice[0].position;
    Vector3 maxCorner = pVertice[0].position;

    for (size_t i = 1; i < numVertice; i++)
    {
        minCorner = Vector3::Min(minCorner, pVertice[i].position);
        maxCorner = Vector3::Max(maxCorner, pVertice[i].position);
    }

    return Bounds(minCorner, maxCorner);
}

Bounds GetBoundingBox(const SkinnedVertex *pVertice, UINT numVertice)
{
    if (numVertice == 0)
        return Bounds();

    Vector3 minCorner = pVertice[0].position;
    Vector3 maxCorner = pVertice[0].position;

    for (size_t i = 1; i < numVertice; i++)
    {
        minCorner = Vector3::Min(minCorner, pVertice[i].position);
        maxCorner = Vector3::Max(maxCorner, pVertice[i].position);
    }

    return Bounds(minCorner, maxCorner);
}

void FootOfAltitude(const Vector3 &a, const Vector3 &b, const Vector3 &p, Vector3 *pOutPoint) 
{
    const Vector3 ab = b - a;
    const Vector3 ac = p - a;
    *pOutPoint = a + ab * (ac.Dot(ab) / ab.LengthSquared());
}

