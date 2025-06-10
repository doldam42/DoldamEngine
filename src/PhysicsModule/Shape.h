#pragma once

#include "../Common/PhysicsInterface.h"

struct Shape
{
    Bounds AABB;
    Matrix InertiaTensor;

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

    virtual Vector3 Support(const Vector3 &dir, const Vector3 &pos, const Quaternion &orient,
                            const float bias) const = 0;
    // virtual float   FastestLinearSpeed(const Vector3& angularVelocity, const Vector3& dir) const = 0;
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

    SphereShape(float radius) : Radius(radius)
    {
        AABB = Bounds(Vector3(-radius), Vector3(radius));
        
        constexpr float oneDiv5 = 1.0f / 5.0f;

        InertiaTensor = Matrix::Identity;
        InertiaTensor._11 = 2.0f * radius * radius * oneDiv5;
        InertiaTensor._22 = 2.0f * radius * radius * oneDiv5;
        InertiaTensor._33 = 2.0f * radius * radius * oneDiv5;
    }
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

        Vector3 maxPt = corners[0];
        float   maxDist = maxPt.Dot(dir);
        for (size_t i = 1; i < _countof(corners); ++i)
        {
            Vector3 pt = corners[i];
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

    BoxShape(const Vector3 &halfExtent) : HalfExtent(halfExtent) 
    { 
        AABB = Bounds(-halfExtent, halfExtent); 

        // Init Tensor
        constexpr float OneDiv12 = 1 / 12.0f;
        // Inertia tensor for box centered around zero
        const float     dx = AABB.WidthX();
        const float     dy = AABB.WidthY();
        const float     dz = AABB.WidthZ();

        Matrix tensor;
        tensor._11 = (dy * dy + dz * dz) * OneDiv12;
        tensor._22 = (dx * dx + dz * dz) * OneDiv12;
        tensor._33 = (dx * dx + dy * dy) * OneDiv12;

        // Now We need to use the parallel axis theorem to get the inertia tensor for a box
        // that is not centered around the origin

        Vector3 cm = AABB.Center();

        const Vector3 R = -cm;
        const float   R2 = R.LengthSquared();

        Matrix patTensor(R2 - R.x * R.x, R.x * R.y, R.x * R.z, 0.0f, R.y * R.x, R2 - R.y * R.y, R.y * R.z, 0.0f,
                         R.z * R.x, R.z * R.y, R2 - R.z * R.z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

        tensor += patTensor;
        InertiaTensor = tensor;
    }
};

/*
  Ÿ��ü. ����(Y Axis)�� ����(X-Z Axis)���� ������. 
  ȸ��X, �� ����
 */
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

        d.y *= scale;
        d *= MinorRadius + bias;
        d.y *= invScale;

        return pos + d;

        /*Vector3 d = dir;
        d.Normalize();

        float minorSquared = MinorRadius * MinorRadius;
        float majorSquared = MajorRadius * MajorRadius;

        Vector3 scaled(minorSquared * d.x, majorSquared * d.y, minorSquared * d.z);

        float denom = sqrtf(minorSquared * d.x * d.x + majorSquared * d.y * d.y + minorSquared * d.z * d.z);

        return pos + (scaled / denom);*/
    }

    EllipsoidShape(float majorRadius, float minorRadius) : MajorRadius(majorRadius), MinorRadius(minorRadius)
    {
        Vector3 extent = Vector3(MinorRadius, MajorRadius, MinorRadius);
        AABB = Bounds(-extent, extent);
    }
};