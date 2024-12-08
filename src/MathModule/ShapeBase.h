#pragma once

enum SHAPE_TYPE
{
    SHAPE_TYPE_SPHERE,
    SHAPE_TYPE_BOX,
};

class Shape
{
  public:
    virtual SHAPE_TYPE GetType() const = 0;

    virtual Bounds GetBounds(const Vector3 &pos, const Quaternion &orient) const = 0;
    virtual Bounds GetBounds() const = 0;

    virtual Vector3 GetCenterOfMass() const { return m_centerOfMass; }
    virtual Matrix  InertiaTensor() const = 0;

  protected:
    Vector3 m_centerOfMass;
};