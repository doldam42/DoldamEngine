#pragma once
#include "EngineInterface.h"

struct CollisionSphere : public ICollisionShape
{
    BoundingSphere sphere;

    void LocalToWorld(CollisionSphere *pOutSphere, const Matrix &worldMat) const
    {
        sphere.Transform(pOutSphere->sphere, worldMat);
    }
    COLLISION_SHAPE_TYPE GetType() const override { return COLLISION_SHAPE_TYPE_SPHERE; }
    CollisionSphere(const Vector3 &center, float radius) : sphere(center, radius) {}
};

struct CollisionBox : public ICollisionShape
{
    BoundingBox box;

    void LocalToWorld(CollisionBox *pOutBox, const Matrix &worldMat) const { box.Transform(pOutBox->box, worldMat); }
    COLLISION_SHAPE_TYPE GetType() const override { return COLLISION_SHAPE_TYPE_BOX; }
    CollisionBox(const Vector3 &center, const Vector3 &extents) : box(center, extents) {}
};


BOOL IsIntersect(const ICollisionShape *pA, const ICollisionShape *pB)
{
    if (pA->GetType() == COLLISION_SHAPE_TYPE_NONE || pB->GetType() == COLLISION_SHAPE_TYPE_NONE)
        return FALSE;

    if (pA->GetType() == COLLISION_SHAPE_TYPE_BOX)
    {
        CollisionBox *pABox = (CollisionBox *)pA;
        if (pB->GetType() == COLLISION_SHAPE_TYPE_BOX)
        {
            CollisionBox *pBBox = (CollisionBox *)pB;
            return pABox->box.Intersects(pBBox->box);
        }
        else
        {
            CollisionSphere *pBSphere = (CollisionSphere *)pB;
            return pABox->box.Intersects(pBSphere->sphere);
        }
    }
    else
    {
        CollisionSphere *pASphere = (CollisionSphere *)pA;
        if (pB->GetType() == COLLISION_SHAPE_TYPE_BOX)
        {
            CollisionBox *pBBox = (CollisionBox *)pB;
            return pASphere->sphere.Intersects(pBBox->box);
        }
        else
        {
            CollisionSphere *pBSphere = (CollisionSphere *)pB;
            return pASphere->sphere.Intersects(pBSphere->sphere);
        }
    }
}
