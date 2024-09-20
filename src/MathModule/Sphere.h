#pragma once

struct Sphere
{
  public:
    Vector3 Center = Vector3::Zero;
    float   Radius = 0.f;

  public:
    Sphere() = default;
    Sphere(const Vector3 &inCenter, const float inRadius) : Center(inCenter), Radius(inRadius) {}

    inline bool IsInside(const Vector3 &inVector) const;
    inline bool Intersect(const Sphere &inSphere) const;
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