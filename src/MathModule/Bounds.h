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
    Bounds() { Clear(); }
    Bounds(const Bounds &rhs) : mins(rhs.mins), maxs(rhs.maxs) {}
    const Bounds &operator=(const Bounds &rhs);
    ~Bounds() {}

    void Clear()
    {
        mins = Vector3(1e6);
        maxs = Vector3(-1e6);
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

  public:
    Vector3 mins;
    Vector3 maxs;
};