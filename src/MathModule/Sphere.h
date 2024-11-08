#pragma once

struct Sphere : public IShape
{
  public:
    Vector3 Center = Vector3::Zero;
    float   Radius = 0.f;

  public:
    Sphere() = default;
    Sphere(const Vector3 &inCenter, const float inRadius) : Center(inCenter), Radius(inRadius) {}

    inline bool IsInside(const Vector3 &inVector) const;
    inline bool Intersect(const Sphere &inSphere) const;
    bool Intersect(const Box &inBox) const;

    void LocalToWorld(Sphere *pOutSphere, const Matrix &worldTM) const;

    // Inheritance via IShape

    inline SHAPE_TYPE GetType() const override { return SHAPE_TYPE_BOX; }

    Vector3 GetCenter() const override { return Center; }
};

bool Sphere::IsInside(const Vector3 &inVector) const
{
    return ((Center - inVector).LengthSquared() <= (Radius * Radius));
}

bool Sphere::Intersect(const Sphere &inSphere) const
{
    float radiusSum = Radius + inSphere.Radius;
    return ((Center - inSphere.Center).LengthSquared() < (radiusSum * radiusSum));
}

//
//bool Sphere::Intersect(const IShape *pOther, Contact *pOutContact) 
//{
//    switch (pOther->GetType())
//    {
//    case SHAPE_TYPE_BOX: {
//        Box *pBox = (Box *)pOther;
//        return CheckCollision(*pBox, *this, pOutContact);
//    }
//    case SHAPE_TYPE_SPHERE: {
//        Sphere *pSphere = (Sphere *)pOther;
//        return CheckCollision(*pSphere, *this, pOutContact);
//    }
//    default:
//        return false;
//    }
//}
