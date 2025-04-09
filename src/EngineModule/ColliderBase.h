#pragma once

interface IColliderBase : public ICollider
{
    virtual COLLIDER_TYPE GetType() const = 0;
    virtual Vector3       GetCenter() const = 0;
    virtual Vector3       GetWorldCenter() const = 0;
    virtual Bounds        GetBounds() const = 0;
    virtual Bounds        GetWorldBounds() const = 0;
    virtual Matrix        InertiaTensor() const = 0;
    virtual BOOL          Intersect(ICollider * pOther) const = 0;
    virtual BOOL          IntersectRay(const Ray &ray, float *hitt0, float *hitt1) const = 0;
    virtual BOOL          Intersect(const Bounds &b) const = 0;

    
    virtual Vector3 Support(const Vector3 dir, const Vector3 pos, const Quaternion orient, const float bias) = 0;
    virtual float    FastestLinearSpeed(const Vector3 angularVelocity, const Vector3 dir) const = 0;
};