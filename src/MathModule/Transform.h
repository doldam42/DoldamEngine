#pragma once

struct Transform
{
  public:
    Transform() = default;
    Transform(const Vector3 &InPosition) : Position(InPosition)
    {
    }
    Transform(const Vector3 &InPosition, const Quaternion &InRotation) : Position(InPosition), Rotation(InRotation)
    {
    }
    Transform(const Vector3 &InPosition, const Quaternion &InRotation, const Vector3 &InScale)
        : Position(InPosition), Rotation(InRotation), Scale(InScale)
    {
    }

  public:
    Transform &operator=(const Transform &other)
    {
        Position = other.GetPosition();
        Rotation = other.GetRotation();
        Scale = other.GetScale();
        return *this;
    }

    void SetPosition(const Vector3 &InPosition)
    {
        Position = InPosition;
    }
    void AddPosition(const Vector3 &InDeltaPosition)
    {
        Position += InDeltaPosition;
    }
    void AddYawRotation(float degree)
    {
        Vector3 r = Rotation.ToEuler();
        r.y += degree;
        Rotation = Quaternion::CreateFromYawPitchRoll(r);
    }
    void AddRollRotation(float degree)
    {
        Vector3 r = Rotation.ToEuler();
        r.z += degree;
        Rotation = Quaternion::CreateFromYawPitchRoll(r);
    }
    void AddPitchRotation(float degree)
    {
        Vector3 r = Rotation.ToEuler();
        r.x += degree;
        Rotation = Quaternion::CreateFromYawPitchRoll(r);
    }

    void SetRotation(const Quaternion &InQuaternion)
    {
        Rotation = InQuaternion;
    }

    Vector3 GetXAxis() const
    {
        return Rotation.ToEuler() * Vector3::UnitX;
    }
    Vector3 GetYAxis() const
    {
        return Rotation.ToEuler() * Vector3::UnitY;
    }
    Vector3 GetZAxis() const
    {
        return Rotation.ToEuler() * Vector3::UnitZ;
    }

    Vector3 GetForward() const
    {
        // Roll은 무시
        Vector3 yawPitchRoll = Rotation.ToEuler();
        yawPitchRoll.y += DirectX::XM_PI;
        // float yaw = Rotation.ToEuler().y;
        // yaw += DirectX::XM_PI;
        return Vector3::Transform(Vector3(0.0f, 0.0f, 1.0f), Matrix::CreateFromYawPitchRoll(yawPitchRoll));
    }

    void SetScale(const Vector3 &InScale)
    {
        Scale = InScale;
    }

    Matrix GetMatrix() const
    {
        return Matrix::CreateScale(Scale) * Matrix::CreateFromQuaternion(Rotation) *
               Matrix::CreateTranslation(Position);
    }

    Vector3 GetPosition() const
    {
        return Position;
    }
    Quaternion GetRotation() const
    {
        return Rotation;
    }
    Vector3 GetScale() const
    {
        return Scale;
    }

    // 트랜스폼 변환

  private:
    Vector3    Position;
    Quaternion Rotation = Quaternion::Identity;
    Vector3    Scale = Vector3::One;
};
