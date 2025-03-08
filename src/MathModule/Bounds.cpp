//
//	Bounds.cpp
//
#include "pch.h"

#include "Bounds.h"

Bounds Bounds::SweptBounds(const Bounds &from, const Bounds &to)
{
    return Bounds(Vector3::Min(from.mins, to.mins), Vector3::Max(from.maxs, to.maxs));
}

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
    __m128 min0 = _mm_set_ps(mins.x, mins.y, mins.z, 0.0f);
    __m128 min1 = _mm_set_ps(rhs.mins.x, rhs.mins.y, rhs.mins.z, 0.0f);
    __m128 max0 = _mm_set_ps(maxs.x, maxs.y, maxs.z, 0.0f);
    __m128 max1 = _mm_set_ps(rhs.maxs.x, rhs.maxs.y, rhs.maxs.z, 0.0f);

    __m128 comp1 = _mm_cmplt_ps(max0, min1);
    __m128 comp2 = _mm_cmplt_ps(max1, min0);

    if (!_mm_testz_ps(comp1, comp1) && !_mm_testz_ps(comp2, comp2))
    {
        return true;
    }
    return false;

    // if (maxs.x < rhs.mins.x || maxs.y < rhs.mins.y || maxs.z < rhs.mins.z)
    //{
    //     return false;
    // }
    // if (rhs.maxs.x < mins.x || rhs.maxs.y < mins.y || rhs.maxs.z < mins.z)
    //{
    //     return false;
    // }
    // return true;
}

bool Bounds::DoesIntersect(const Vector3 &center, const float radius) const 
{
    __m128 _center = _mm_set_ps(center.x, center.y, center.z, 0.0f);
    __m128 _mins = _mm_set_ps(mins.x, mins.y, mins.z, 0.0f);
    __m128 _maxs = _mm_set_ps(maxs.x, maxs.y, maxs.z, 0.0f);

    __m128 _closest = _mm_max_ps(_mins, _mm_min_ps(_center, _maxs)); 
    __m128 _d = _mm_sub_ps(_closest, _center);
    __m128 _d2 = _mm_dp_ps(_d, _d, 0xF1);

    float  distSquared;
    _mm_store_ss(&distSquared, _d2);

    return distSquared <= (radius * radius);

    //// AABB 내부에서 Sphere 중심과 가장 가까운 점 찾기
    //Vector3 closestPoint;
    //closestPoint.x = std::fmax(mins.x, std::fmin(center.x, maxs.x));
    //closestPoint.y = std::fmax(mins.y, std::fmin(center.y, maxs.y));
    //closestPoint.z = std::fmax(mins.z, std::fmin(center.z, maxs.z));

    //// 가장 가까운 점과 Sphere 중심 간 거리 계산
    //float dx = closestPoint.x - center.x;
    //float dy = closestPoint.y - center.y;
    //float dz = closestPoint.z - center.z;
    //float distanceSquared = dx * dx + dy * dy + dz * dz;

    //// 거리가 Sphere 반지름 이하이면 충돌
    //return distanceSquared <= (radius * radius);
}

bool Bounds::IntersectP(const Ray &ray, float *hitt0, float *hitt1) const
{
    __m128 origin = _mm_set_ps(ray.position.x, ray.position.y, ray.position.z, 0.0f);
    __m128 invDir = _mm_set_ps(1 / ray.direction.x, 1 / ray.direction.y, 1 / ray.direction.z, 0.0f);

    __m128 boundsMin = _mm_set_ps(mins.x, mins.y, mins.z, 0.0f);
    __m128 boundsMax = _mm_set_ps(maxs.x, maxs.y, maxs.z, 0.0f);

    __m128 t1 = _mm_mul_ps(_mm_sub_ps(boundsMin, origin), invDir);
    __m128 t2 = _mm_mul_ps(_mm_sub_ps(boundsMax, origin), invDir);

    __m128 tminVec = _mm_min_ps(t1, t2);
    __m128 tmaxVec = _mm_max_ps(t1, t2);

    float fMinVec[4];
    float fMaxVec[4];

    _mm_store_ps(fMinVec, tminVec);
    _mm_store_ps(fMaxVec, tmaxVec);

    float tmin = max(max(fMinVec[1], fMinVec[2]), fMinVec[3]);
    float tmax = min(min(fMaxVec[1], fMaxVec[2]), fMaxVec[3]);

    if (tmax >= tmin && tmax >= 0 && tmin <= ray.tmax)
    {
        *hitt0 = tmin;
        *hitt1 = tmax;
        return true;
    }
    return false;
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
    mins = Vector3::Min(mins, rhs);
    maxs = Vector3::Max(maxs, rhs);
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

Vector3 Bounds::Center() const
{
    using namespace DirectX;
    return XMVectorScale(XMVectorAdd(mins, maxs), 0.5f);
}

Vector3 Bounds::Extends() const
{
    using namespace DirectX;
    return XMVectorScale(XMVectorSubtract(maxs, mins), 0.5f);
}

Vector3 Bounds::Offset(const Vector3 &p) const
{
    Vector3 o = p - mins;
    if (maxs.x > mins.x)
        o.x /= maxs.x - mins.x;
    if (maxs.y > mins.y)
        o.y /= maxs.y - mins.y;
    if (maxs.z > mins.z)
        o.z /= maxs.z - mins.z;
    return o;
}

void Bounds::Transform(Bounds *pOutBounds, const Matrix &m) const
{
    using namespace DirectX;
    // Load center and extents.
    XMVECTOR vCenter = XMVectorScale(XMVectorAdd(mins, maxs), 0.5f);
    XMVECTOR vExtents = XMVectorScale(XMVectorSubtract(maxs, mins), 0.5f);

    // Compute and transform the corners and find new min/max bounds.
    XMVECTOR Corner = XMVectorMultiplyAdd(vExtents, g_BoxOffset[0], vCenter);
    Corner = XMVector3Transform(Corner, m);

    XMVECTOR Min, Max;
    Min = Max = Corner;

    for (size_t i = 1; i < CORNER_COUNT; ++i)
    {
        Corner = XMVectorMultiplyAdd(vExtents, g_BoxOffset[i], vCenter);
        Corner = XMVector3Transform(Corner, m);

        Min = XMVectorMin(Min, Corner);
        Max = XMVectorMax(Max, Corner);
    }

    XMStoreFloat3(&pOutBounds->mins, Min);
    XMStoreFloat3(&pOutBounds->maxs, Max);
}
