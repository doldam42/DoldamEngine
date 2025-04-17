#pragma once

struct RigidBody;
struct Collider : public ICollider
{
    IGameObject *pObj = nullptr;
    RigidBody *pBody = nullptr;

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

    virtual COLLIDER_TYPE GetType() const = 0;
    virtual BOOL          RayTest(const Vector3 rayStart, const Vector3 &rayDir, float *tHit) = 0;

    BOOL IsCollisionEnter() override { return (!IsPrevCollide && IsCollide); }
    BOOL IsCollisionStay() override { return (IsPrevCollide && IsCollide); }
    BOOL IsCollisionExit() override { return (IsPrevCollide && !IsCollide); }

    IGameObject *GetGameObject() override { return pObj; }
};
