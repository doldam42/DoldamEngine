#pragma once

class Bounds
{
  public:
    Vector3 Min;
    Vector3 Max;

  public:
    Bounds() { Clear(); };
    Bounds(const Bounds &rhs) : Min(rhs.Min), Max(rhs.Max) {}
    const Bounds &operator=(const Bounds &rhs);
    
    void Clear()
    {
        Min = Vector3(FLT_MAX);
        Max = Vector3(FLT_MIN);
    }

    bool DoesIntersect(const Bounds &rhs) const;
    void Expand(const Vector3 *points, const UINT numPoints);
    void Expand(const Vector3 &rhs);
    void Expand(const Bounds &rhs);

    float WidthX() const { return Max.x - Min.x; }
    float WidthY() const { return Max.y - Min.y; }
    float WidthZ() const { return Max.z - Min.z; }
};