//
//	Bounds.h
//
#pragma once

/*
====================================================
Bounds
====================================================
*/
class Bounds
{
  public:
    static constexpr size_t CORNER_COUNT = 8;

    static Bounds SweptBounds(const Bounds &from, const Bounds &to);

    Bounds() { Clear(); }
    Bounds(const Bounds &rhs) : mins(rhs.mins), maxs(rhs.maxs) {}
    Bounds(const Vector3 &minCorner, const Vector3 &maxCorner) : mins(minCorner), maxs(maxCorner) {}

    const Bounds &operator=(const Bounds &rhs);
    ~Bounds() {}

    void Clear()
    {
        mins = Vector3(FLT_MAX);
        maxs = Vector3(FLT_MIN);
    }
    bool DoesIntersect(const Bounds &rhs) const;
    bool IntersectP(const Ray &ray, float *hitt0, float *hitt1);

    void Expand(const Vector3 *pts, const int num);
    void Expand(const Vector3 &rhs);
    void Expand(const Bounds &rhs);

    float WidthX() const { return maxs.x - mins.x; }
    float WidthY() const { return maxs.y - mins.y; }
    float WidthZ() const { return maxs.z - mins.z; }

    Vector3 Diagonal() const;
    float   SurfaceArea() const;
    int     MaximumExtent() const;

    Vector3 Center() const;
    Vector3 Extends() const;

    void Transform(Bounds *pOutBounds, const Matrix m) const;

  public:
    Vector3 mins;
    Vector3 maxs;
};