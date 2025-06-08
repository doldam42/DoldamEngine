#pragma once

#include "../Common/PhysicsInterface.h"

struct Shape
{
    Bounds AABB;

    Bounds GetBounds() const { return AABB; }
    Bounds GetWorldBounds(const Vector3 &pos, const Quaternion &orient) const
    {
        Bounds b;
        AABB.Transform(&b, pos, orient);
        return b;
    }

    virtual SHAPE_TYPE GetType() const = 0;

    virtual BOOL RayTest(const Ray &ray, const Vector3 &pos, const Quaternion &orient, Vector3 *pOutNormal,
                         float *tHit) = 0;

    virtual Vector3 Support(const Vector3& dir, const Vector3& pos, const Quaternion& orient, const float bias) const = 0;
    //virtual float   FastestLinearSpeed(const Vector3& angularVelocity, const Vector3& dir) const = 0;
};

struct SphereShape : public Shape
{
    float Radius;

    SHAPE_TYPE GetType() const override { return SHAPE_TYPE_SPHERE; }

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

    Vector3 Support(const Vector3 &dir, const Vector3 &pos, const Quaternion &orient, const float bias) const override
    {
        return pos + dir * (Radius + bias);
    }

    SphereShape(float radius) : Radius(radius) { AABB = Bounds(Vector3(-radius), Vector3(radius)); }
};

struct BoxShape : public Shape
{
    static constexpr UINT CORNER_COUNT = 8;

    Vector3 HalfExtent;

    SHAPE_TYPE GetType() const override { return SHAPE_TYPE_BOX; }

    void GetCorners(const Vector3 &pos, const Quaternion &orient, Vector3 pOutCorners[8]) const
    {
        AABB.GetCorners(pOutCorners);
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

    Vector3 Support(const Vector3 &dir, const Vector3 &pos, const Quaternion &orient, const float bias) const override
    {
        Vector3 corners[8];
        GetCorners(pos, orient, corners);

        Vector3 maxPt = Vector3::Transform(corners[0], orient) + pos;
        float   maxDist = maxPt.Dot(dir);
        for (size_t i = 1; i < 8; ++i)
        {
            Vector3 pt = Vector3::Transform(corners[i], orient) + pos;
            float   dist = pt.Dot(dir);
            if (dist > maxDist)
            {
                maxDist = dist;
                maxPt = pt;
            }
        }

        Vector3 norm = dir;
        norm.Normalize();
        norm *= bias;

        return maxPt + norm;
    }

    BoxShape(const Vector3 &halfExtent) : HalfExtent(halfExtent) { AABB = Bounds(-halfExtent, halfExtent); }
};

struct EllipsoidShape : public Shape
{
    float MajorRadius; // Y axis
    float MinorRadius; // X-Z axis

    SHAPE_TYPE GetType() const override { return SHAPE_TYPE_ELLIPSOID; }

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

    Vector3 Support(const Vector3 &dir, const Vector3 &pos, const Quaternion &orient, const float bias) const override
    {
        const float scale = MinorRadius / MajorRadius;
        const float invScale = 1.0f / scale;

        Vector3 d = dir;
        Vector3 p = pos;

        d.y *= scale;
        p.y *= scale;

        Vector3 ret = p + d * (MinorRadius + bias);

        ret.y *= invScale;

        return ret;
    }

    EllipsoidShape(float majorRadius, float minorRadius) : MajorRadius(majorRadius), MinorRadius(minorRadius)
    {
        Vector3 extent = Vector3(MinorRadius, MajorRadius, MinorRadius);
        AABB = Bounds(-extent, extent);
    }
};