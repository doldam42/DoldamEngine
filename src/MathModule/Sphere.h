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

    Bounds GetBounds(const Vector3 &pos, const Quaternion &orient) const override;
    Bounds GetBounds() const override;

    Matrix InertiaTensor() const override
    {
        Matrix tensor = Matrix::Identity;
        tensor.m[0][0] = 2.0f * Radius * Radius / 5.0f;
        tensor.m[1][1] = 2.0f * Radius * Radius / 5.0f;
        tensor.m[2][2] = 2.0f * Radius * Radius / 5.0f;

        return tensor;
    }
};
