#pragma once

struct Collider : public ICollider
{
    UINT ID;
    BOOL IsActive;

    Vector3    Position;
    Quaternion Rotation;

    Vector3    GetPosition() override { return Position; }
    Quaternion GetRotation() override { return Rotation; }

    void SetPosition(const Vector3 &pos) override { Position = pos; }
    void SetRotation(const Quaternion &q) override { Rotation = q; }
    void SetActive(BOOL isActive) override { IsActive = isActive; }

    virtual COLLIDER_TYPE GetType() const = 0;
    virtual Bounds        GetBounds() const = 0;
};
