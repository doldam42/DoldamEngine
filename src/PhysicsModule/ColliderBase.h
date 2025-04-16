#pragma once

struct RigidBody;
struct Collider : public ICollider
{
    RigidBody *pBody = nullptr;

    UINT ID;
    BOOL IsActive;
    BOOL IsCollide;
    BOOL IsPrevCollide;

    Vector3    Position;
    Quaternion Rotation;

    Vector3    GetPosition() override { return Position; }
    Quaternion GetRotation() override { return Rotation; }

    void SetPosition(const Vector3 &pos) override { Position = pos; }
    void SetRotation(const Quaternion &q) override { Rotation = q; }
    void SetActive(BOOL isActive) override { IsActive = isActive; }

    virtual COLLIDER_TYPE GetType() const = 0;
    virtual Bounds        GetBounds() const = 0;

    BOOL IsCollisionEnter() override { return (!IsPrevCollide && IsCollide); }
    BOOL IsCollisionStay() override { return (IsPrevCollide && IsCollide); }
    BOOL IsCollisionExit() override { return (IsPrevCollide && !IsCollide); }
};
