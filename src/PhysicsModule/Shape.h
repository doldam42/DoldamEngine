#pragma once

#include "../Common/PhysicsInterface.h"

interface IShape
{
    virtual SHAPE_TYPE GetType() const = 0;

    virtual Bounds GetBounds() const = 0;
    virtual Bounds GetWorldBounds(const Vector3 &pos, const Quaternion &orient) const = 0;

    virtual BOOL RayTest(const Ray &ray, const Vector3 &pos, const Quaternion &orient, Vector3 *pOutNormal,
                         float *tHit) = 0;
};

struct SphereShape : public IShape
{
    float Radius;

    SHAPE_TYPE GetType() const override { return SHAPE_TYPE_SPHERE; }

    Bounds GetBounds() const override { return Bounds(Vector3(-Radius), Vector3(Radius)); }

    Bounds GetWorldBounds(const Vector3 &pos, const Quaternion &orient) const override
    {
        Vector3 mins = pos - Vector3(Radius);
        Vector3 maxs = pos + Vector3(Radius);
        return Bounds(mins, maxs);
    }

    BOOL RayTest(const Ray &ray, const Vector3 &pos, const Quaternion &orient, Vector3 *pOutNormal,
                 float *tHit) override
    {
        Vector3 normal;
        float   t0, t1;
        if (RaySphere(ray.position, ray.direction, pos, Radius, &normal, &t0, &t1))
        {
            *pOutNormal = normal;
            *tHit = t0;
            return TRUE;
        }
        return FALSE;
    }

    SphereShape(float radius) : Radius(radius) {}
};

struct BoxShape : public IShape
{
    static constexpr UINT CORNER_COUNT = 8;

    Vector3 HalfExtent;

    SHAPE_TYPE GetType() const override { return SHAPE_TYPE_BOX; }

    Bounds GetBounds() const override { return Bounds(-HalfExtent, HalfExtent); }

    Bounds GetWorldBounds(const Vector3 &pos, const Quaternion &orient) const override
    {
        Vector3 mins = pos - HalfExtent;
        Vector3 maxs = pos + HalfExtent;
        return Bounds(mins, maxs);
    }

    void GetCorners(const Vector3 &pos, const Quaternion &orient, Vector3 pOutCorners[8]) const
    {
        Bounds b(-HalfExtent, HalfExtent);

        b.GetCorners(pOutCorners);
        for (int i = 0; i < CORNER_COUNT; i++)
        {
            pOutCorners[i] = Vector3::Transform(pOutCorners[i], orient) + pos;
        }
    }

    BOOL RayTest(const Ray &ray, const Vector3 &pos, const Quaternion &orient, Vector3 *pOutNormal,
                 float *tHit) override
    {
        Vector3 normal;
        float   hitt;
        if (RayBox(ray.position, ray.direction, pos, HalfExtent, orient, &normal, &hitt))
        {
            *tHit = hitt;
            *pOutNormal = normal;
            return TRUE;
        }
        return FALSE;
    }

    BoxShape(const Vector3 &halfExtent) : HalfExtent(halfExtent) {}
};

struct EllipsoidShape : public IShape
{
    float MajorRadius; // Y axis
    float MinorRadius; // X-Z axis

    SHAPE_TYPE GetType() const override { return SHAPE_TYPE_ELLIPSOID; }

    Bounds GetBounds() const override
    {
        Vector3 extent = Vector3(MinorRadius, MajorRadius, MinorRadius);
        return Bounds(-extent, extent);
    }

    Bounds GetWorldBounds(const Vector3 &pos, const Quaternion &orient) const override
    {
        Vector3 extent = Vector3(MinorRadius, MajorRadius, MinorRadius);
        Vector3 mins = pos - extent;
        Vector3 maxs = pos + extent;
        return Bounds(mins, maxs);
    }

    BOOL RayTest(const Ray &ray, const Vector3 &pos, const Quaternion &orient, Vector3 *pOutNormal,
                 float *tHit) override
    {
        Vector3 normal;
        float   hitt0, hitt1;
        if (RayEllipse(ray.position, ray.direction, pos, MajorRadius, MinorRadius, &normal, &hitt0, &hitt1) &&
            hitt0 > 0.0f)
        {
            *pOutNormal = normal;
            *tHit = hitt0;
            return TRUE;
        }
        return FALSE;
    }

    EllipsoidShape(float majorRadius, float minorRadius) : MajorRadius(majorRadius), MinorRadius(minorRadius) {}
};