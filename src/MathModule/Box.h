#pragma once

struct Box
{
  public:
    Vector3 Min;
    Vector3 Max;

  public:
    inline bool Intersect(const Box &inBox) const;
    inline bool IsInside(const Box &inBox) const;
    inline bool IsInside(const Vector3 &inVector) const;

    inline Vector3 GetSize() const;
    inline Vector3 GetExtent() const;
    inline Vector3 GetCenter() const;

    Box() = default;
    Box(const Box &inBox) : Min(inBox.Min), Max(inBox.Max) {}
    Box(const Vector3 &inMin, const Vector3 &inMax) : Min(inMin), Max(inMax) {}
};

bool Box::Intersect(const Box &inBox) const 
{
    if ((Min.x > inBox.Max.x) || (inBox.Min.x > Max.x))
    {
        return false;
    }

    if ((Min.y > inBox.Max.y) || (inBox.Min.y > Max.y))
    {
        return false;
    }

    if ((Min.z > inBox.Max.z) || (inBox.Min.z > Max.z))
    {
        return false;
    }

    return true;
}

bool Box::IsInside(const Box &inBox) const { return (IsInside(inBox.Min) && IsInside(inBox.Max)); }

bool Box::IsInside(const Vector3 &inVector) const
{
    return ((inVector.x >= Min.x) && (inVector.x <= Max.x) && (inVector.y >= Min.y) && (inVector.y <= Max.y) &&
            (inVector.z >= Min.z) && (inVector.z <= Max.z));
}

Vector3 Box::GetSize() const { return (Max - Min); }

Vector3 Box::GetCenter() const { return Min + GetExtent(); }

Vector3 Box::GetExtent() const { return GetSize() * 0.5f; }