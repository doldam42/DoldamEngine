//
//	Bounds.cpp
//
#include "pch.h"

#include "Bounds.h"

/*
====================================================
Bounds::operator =
====================================================
*/
const Bounds &Bounds::operator=(const Bounds &rhs)
{
    mins = rhs.mins;
    maxs = rhs.maxs;
    return *this;
}

/*
====================================================
Bounds::DoesIntersect
====================================================
*/
bool Bounds::DoesIntersect(const Bounds &rhs) const
{
    if (maxs.x < rhs.mins.x || maxs.y < rhs.mins.y || maxs.z < rhs.mins.z)
    {
        return false;
    }
    if (rhs.maxs.x < mins.x || rhs.maxs.y < mins.y || rhs.maxs.z < mins.z)
    {
        return false;
    }
    return true;
}

bool Bounds::IntersectP(const Ray &ray, float *hitt0, float *hitt1) 
{
    float t0 = 0, t1 = FLT_MAX;

    float *pMin = (float *)&mins;
    float *pMax = (float *)&maxs;
    float *pRayPos = (float *)&ray.pos;
    float *pRayDir = (float *)&ray.dir;
    for (int i = 0; i < 3; i++)
    {
        // Update interval for _i_th bounding box slab
        float invRayDir = 1 / pRayDir[i];
        float tNear = (pMin[i] - pRayPos[i]) * invRayDir;
        float tFar = (pMax[i] - pRayPos[i]) * invRayDir;

        // Update parametric interval from slab intersection $t$ values
        if (tNear > tFar)
            std::swap(tNear, tFar);

        // Update _tFar_ to ensure robust ray--bounds intersection
        tFar *= 1 + 2 * gamma(3);
        t0 = tNear > t0 ? tNear : t0;
        t1 = tFar < t1 ? tFar : t1;
        if (t0 > t1)
            return false;
        
    }
    if (hitt0)
        *hitt0 = t0;
    if (hitt1)
        *hitt1 = t1;
    return true;
}

/*
====================================================
Bounds::Expand
====================================================
*/
void Bounds::Expand(const Vector3 *pts, const int num)
{
    for (int i = 0; i < num; i++)
    {
        Expand(pts[i]);
    }
}

/*
====================================================
Bounds::Expand
====================================================
*/
void Bounds::Expand(const Vector3 &rhs)
{
    if (rhs.x < mins.x)
    {
        mins.x = rhs.x;
    }
    if (rhs.y < mins.y)
    {
        mins.y = rhs.y;
    }
    if (rhs.z < mins.z)
    {
        mins.z = rhs.z;
    }

    if (rhs.x > maxs.x)
    {
        maxs.x = rhs.x;
    }
    if (rhs.y > maxs.y)
    {
        maxs.y = rhs.y;
    }
    if (rhs.z > maxs.z)
    {
        maxs.z = rhs.z;
    }
}

/*
====================================================
Bounds::Expand
====================================================
*/
void Bounds::Expand(const Bounds &rhs)
{
    Expand(rhs.mins);
    Expand(rhs.maxs);
}

Vector3 Bounds::Diagonal() const { return maxs - mins; }

float Bounds::SurfaceArea() const
{
    Vector3 d = Diagonal();
    return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
}

int Bounds::MaximumExtent() const
{
    Vector3 d = Diagonal();
    if (d.x > d.y && d.x > d.z)
        return 0;
    else if (d.y > d.z)
        return 1;
    else
        return 2;
}
