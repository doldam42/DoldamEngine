//
//	Bounds.cpp
//
#include "pch.h"

#include "Bounds.h"

Bounds Bounds::SweptBounds(const Bounds &from, const Bounds &to)
{
    return Bounds(Vector3::Min(from.mins, to.mins), Vector3::Max(from.maxs, to.maxs));
}

BOOL Bounds::DoesIntersect(const Bounds &a, const Bounds &b, const Vector3 &velA, const Vector3 &velB, float *hitt0,
                           float *hitt1)
{
    using namespace DirectX;

    __m128 a_min = _mm_set_ps(0, a.mins.z, a.mins.y, a.mins.x);
    __m128 a_max = _mm_set_ps(0, a.maxs.z, a.maxs.y, a.maxs.x);
    __m128 b_min = _mm_set_ps(0, b.mins.z, b.mins.y, b.mins.x);
    __m128 b_max = _mm_set_ps(0, b.maxs.z, b.maxs.y, b.maxs.x);

    __m128 vA = _mm_set_ps(0, velA.z, velA.y, velA.x);
    __m128 vB = _mm_set_ps(0, velB.z, velB.y, velB.x);
    __m128 relVel = _mm_sub_ps(vA, vB); // 상대 속도 계산

    __m128 zero = _mm_setzero_ps();

    // invEntry = (bMin - aMax) / v, invExit = (bMax - aMin) / v
    __m128 invEntry = _mm_div_ps(_mm_sub_ps(b_min, a_max), relVel);
    __m128 invExit = _mm_div_ps(_mm_sub_ps(b_max, a_min), relVel);

    // relVel < 0 -> swap(invEntry, invExit)
    __m128 mask = _mm_cmplt_ps(relVel, zero);
    __m128 temp = invEntry;
    invEntry = _mm_blendv_ps(invEntry, invExit, mask);
    invExit = _mm_blendv_ps(invExit, temp, mask);

    // tMin = max(invEntry.x, invEntry.y, invEntry.z)
    __m128 tMinVec = _mm_max_ps(invEntry, _mm_shuffle_ps(invEntry, invEntry, _MM_SHUFFLE(0, 0, 2, 1)));
    tMinVec = _mm_max_ps(tMinVec, _mm_shuffle_ps(tMinVec, tMinVec, _MM_SHUFFLE(0, 0, 0, 2)));
    float tMin = _mm_cvtss_f32(tMinVec);

    // tMax = min(invExit.x, invExit.y, invExit.z)
    __m128 tMaxVec = _mm_min_ps(invExit, _mm_shuffle_ps(invExit, invExit, _MM_SHUFFLE(0, 0, 2, 1)));
    tMaxVec = _mm_min_ps(tMaxVec, _mm_shuffle_ps(tMaxVec, tMaxVec, _MM_SHUFFLE(0, 0, 0, 2)));
    float tMax = _mm_cvtss_f32(tMaxVec);

    if (tMin > tMax || tMin > 1.0f || tMin < 0.0f)
        return false;

    *hitt0 = tMin;
    *hitt1 = tMax;

    return true;
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
    Vector3 closestPoint = Vector3::Max(mins, Vector3::Min(center, maxs));
    Vector3 d = closestPoint - center;
    return d.LengthSquared() <= (radius * radius);

    /*__m128 _center = _mm_set_ps(center.x, center.y, center.z, 0.0f);
    __m128 _mins = _mm_set_ps(mins.x, mins.y, mins.z, 0.0f);
    __m128 _maxs = _mm_set_ps(maxs.x, maxs.y, maxs.z, 0.0f);

    __m128 _closest = _mm_max_ps(_mins, _mm_min_ps(_center, _maxs));
    __m128 _d = _mm_sub_ps(_closest, _center);
    __m128 _d2 = _mm_dp_ps(_d, _d, 0xF1);

    float distSquared;
    _mm_store_ss(&distSquared, _d2);

    return distSquared <= (radius * radius);*/

    //// AABB 내부에서 Sphere 중심과 가장 가까운 점 찾기
    // Vector3 closestPoint;
    // closestPoint.x = std::fmax(mins.x, std::fmin(center.x, maxs.x));
    // closestPoint.y = std::fmax(mins.y, std::fmin(center.y, maxs.y));
    // closestPoint.z = std::fmax(mins.z, std::fmin(center.z, maxs.z));

    //// 가장 가까운 점과 Sphere 중심 간 거리 계산
    // float dx = closestPoint.x - center.x;
    // float dy = closestPoint.y - center.y;
    // float dz = closestPoint.z - center.z;
    // float distanceSquared = dx * dx + dy * dy + dz * dz;

    //// 거리가 Sphere 반지름 이하이면 충돌
    // return distanceSquared <= (radius * radius);
}

bool Bounds::IntersectP(const Ray &ray, float *hitt0, float *hitt1) const
{
    const Vector3 invDir = Vector3(1.0f / ray.direction.x, 1.0f / ray.direction.y, 1.0f / ray.direction.z);

    Vector3 t1 = (mins - ray.position) * invDir;
    Vector3 t2 = (maxs - ray.position) * invDir;

    Vector3 _min = Vector3::Min(t1, t2);
    Vector3 _max = Vector3::Max(t1, t2);
    float   tmin = max(max(_min.x, _min.y), _min.z);
    float   tmax = min(min(_max.x, _max.y), _max.z);

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
    return (mins + maxs) * 0.5f;
}

Vector3 Bounds::Extends() const
{
    return (maxs - mins) * 0.5f;
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

void Bounds::GetCorners(Vector3 pOutCorners[8]) const
{
    for (int i = 0; i < CORNER_COUNT; i++)
    {
        pOutCorners[i] = {
            (i & 0b100) ? maxs.x : mins.x, // x 결정
            (i & 0b010) ? maxs.y : mins.y, // y 결정
            (i & 0b001) ? maxs.z : mins.z  // z 결정
        };
    }
}

void Bounds::Transform(Bounds *pOutBounds, const Matrix &m) const
{
    Vector3 corners[8];
    GetCorners(corners);

    Vector3 corner = Vector3::Transform(corners[0], m);

    Vector3 _min, _max;
    _min = _max = corner;

    for (int i = 1; i < CORNER_COUNT; i++)
    {
        corner = Vector3::Transform(corners[i], m);
        _min = Vector3::Min(_min, corner);
        _max = Vector3::Max(_max, corner);
    }

    pOutBounds->mins = _min;
    pOutBounds->maxs = _max;
}

void Bounds::Transform(Bounds *pOutBounds, const Vector3 &pos, const Quaternion &orient) const 
{
    const Matrix m = Matrix::CreateFromQuaternion(orient);

    Vector3 corners[8];
    GetCorners(corners);

    Vector3 corner = Vector3::Transform(corners[0], m) + pos;

    Vector3 _min, _max;
    _min = _max = corner;

    for (int i = 1; i < CORNER_COUNT; i++)
    {
        corner = Vector3::Transform(corners[i], m) + pos;
        _min = Vector3::Min(_min, corner);
        _max = Vector3::Max(_max, corner);
    }

    pOutBounds->mins = _min;
    pOutBounds->maxs = _max;
}
