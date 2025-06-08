#pragma once

#include "Shape.h"

struct RigidBody;
struct Collider : public ICollider
{
    IGameObject *pObj = nullptr;
    RigidBody   *pBody = nullptr;
    IShape      *pShape = nullptr;

    UINT ID = 0;
    BOOL IsActive = TRUE;
    BOOL IsCollide = FALSE;
    BOOL IsPrevCollide = FALSE;

    Vector3    Position;
    Quaternion Rotation;

    Vector3    GetPosition() override { return Position; }
    Quaternion GetRotation() override { return Rotation; }

    void SetPosition(const Vector3 &pos) override { Position = pos; }
    void SetRotation(const Quaternion &q) override { Rotation = q; }
    void SetActive(BOOL isActive) override { IsActive = isActive; }

    BOOL IsCollisionEnter() override { return (!IsPrevCollide && IsCollide); }
    BOOL IsCollisionStay() override { return (IsPrevCollide && IsCollide); }
    BOOL IsCollisionExit() override { return (IsPrevCollide && !IsCollide); }

    Bounds GetBounds() const override { return pShape->GetWorldBounds(Position, Rotation); }

    UINT GetCollidingColliders(ICollider **ppOutColliders, UINT maxColliders = 7) override;
    UINT QueryCollisionData(CollisionData **ppOutData, UINT maxCollision = 7) override;

    IGameObject *GetGameObject() override { return pObj; }

    BOOL RayTest(const Ray &ray, Vector3 *pOutNormal, float *tHit)
    {
        return pShape->RayTest(ray, Position, Rotation, pOutNormal, tHit);
    }

    Collider() = default;
    ~Collider() { delete pShape; }
};
