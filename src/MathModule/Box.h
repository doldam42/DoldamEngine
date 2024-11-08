#pragma once

struct Box : public IShape
{
  public:
    static const UINT CORNER_COUNT = 8;
    Vector3           Min;
    Vector3           Max;

  public:
    inline bool Intersect(const Box &inBox) const;
    inline bool IsInside(const Box &inBox) const;
    inline bool IsInside(const Vector3 &inVector) const;

    inline Vector3 GetSize() const;
    inline Vector3 GetExtent() const;

    inline void GetCenterAndExtent(Vector3 *pOutCenter, Vector3 *pOutExtent) const;
    void LocalToWorld(Box *pOutBox, const Matrix &worldTM) const;

    inline Vector3 GetClosestPoint(const Vector3 &point) const;

    Box() = default;
    Box(const Box &inBox) : Min(inBox.Min), Max(inBox.Max) {}
    Box(const Vector3 &inMin, const Vector3 &inMax) : Min(inMin), Max(inMax) {}

    // Inheritance via IShape
    inline Vector3    GetCenterOfMass() const override;
    inline SHAPE_TYPE GetType() const override { return SHAPE_TYPE_BOX; }
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
//
//inline bool Box::Intersect(const IShape *pOther, Contact *pOutContact)
//{
//    switch (pOther->GetType())
//    {
//    case SHAPE_TYPE_BOX: {
//        Box *pBox = (Box *)pOther;
//        // return CheckCollision(*pBox, *this, pOutContact);
//        return pBox->Intersect(*this);
//    }
//    case SHAPE_TYPE_SPHERE: {
//        Sphere *pSphere = (Sphere *)pOther;
//        return CheckCollision(*this, *pSphere, pOutContact);
//    }
//    default:
//        return false;
//    }
//}

Vector3 Box::GetCenterOfMass() const { return (Max + Min) * 0.5f; }

Vector3 Box::GetExtent() const { return GetSize() * 0.5f; }

void Box::GetCenterAndExtent(Vector3 *pOutCenter, Vector3 *pOutExtent) const
{
    *pOutExtent = GetExtent();
    *pOutCenter = GetCenterOfMass();
}

Vector3 Box::GetClosestPoint(const Vector3 &point) const
{
    // using namespace DirectX;
    // XMVECTOR SphereCenter = XMLoadFloat3(&point);

    // Vector3 Center;
    // Vector3 Extents;
    // GetCenterAndExtent(&Center, &Extents);

    //// Load center and extents.
    // XMVECTOR BoxCenter = XMLoadFloat3(&Center);
    // XMVECTOR BoxExtents = XMLoadFloat3(&Extents);

    // XMVECTOR BoxMin = XMVectorSubtract(BoxCenter, BoxExtents);
    // XMVECTOR BoxMax = XMVectorAdd(BoxCenter, BoxExtents);

    //// Find the distance to the nearest point on the box.
    //// for each i in (x, y, z)
    //// if (SphereCenter(i) < BoxMin(i)) d2 += (SphereCenter(i) - BoxMin(i)) ^ 2
    //// else if (SphereCenter(i) > BoxMax(i)) d2 += (SphereCenter(i) - BoxMax(i)) ^ 2

    // XMVECTOR d = XMVectorZero();

    //// Compute d for each dimension.
    // XMVECTOR LessThanMin = XMVectorLess(SphereCenter, BoxMin);
    // XMVECTOR GreaterThanMax = XMVectorGreater(SphereCenter, BoxMax);

    // XMVECTOR MinDelta = XMVectorSubtract(SphereCenter, BoxMin);
    // XMVECTOR MaxDelta = XMVectorSubtract(SphereCenter, BoxMax);

    //// Choose value for each dimension based on the comparison.
    // d = XMVectorSelect(d, MinDelta, LessThanMin);
    // d = XMVectorSelect(d, MaxDelta, GreaterThanMax);

    // Vector3 normal = Center - point;
    // normal.Normalize();

    // Vector3 closestPoint = point + normal * Vector3(d).Length();

    Vector3 closestPoint = point;
    closestPoint.Clamp(Min, Max);
    return closestPoint;
}