#pragma once

struct Sphere : public Shape
{
  public:
    float Radius = 0.f;

  public:
    // Inherited via Shape
    explicit Sphere(const float radius) : Radius(radius) { m_centerOfMass = Vector3::Zero; }
    Sphere(const Sphere &rhs) : Radius(rhs.Radius) { m_centerOfMass = rhs.m_centerOfMass; }

    SHAPE_TYPE GetType() const override { return SHAPE_TYPE_SPHERE; }
};