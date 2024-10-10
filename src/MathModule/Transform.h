#pragma once

struct Transform
{
  public:
    Transform() = default;
    Transform(const Vector3 &InPosition) : Position(InPosition) {}
    Transform(const Vector3 &InPosition, const Quaternion &InRotation) : Position(InPosition), Rotation(InRotation) {}
    Transform(const Vector3 &InPosition, const Quaternion &InRotation, const Vector3 &InScale)
        : Position(InPosition), Rotation(InRotation), Scale(InScale)
    {
    }

    // DirectX ÁÂÇ¥°è
    Transform(const Matrix &InMatrix)
    {
        Position = Vector3(InMatrix._41, InMatrix._42, InMatrix._43);
        
        float sx = Vector3(InMatrix._11, InMatrix._12, InMatrix._13).Length();
        float sy = Vector3(InMatrix._21, InMatrix._22, InMatrix._23).Length();
        float sz = Vector3(InMatrix._31, InMatrix._32, InMatrix._33).Length();

        Scale = Vector3(sx, sy, sz);

        Matrix rot = InMatrix;
        rot._41 = rot._42 = rot._43 = 0;
        rot._11 /= sx;
        rot._12 /= sx;
        rot._13 /= sx;
        rot._21 /= sy;
        rot._22 /= sy;
        rot._23 /= sy;
        rot._31 /= sz;
        rot._32 /= sz;
        rot._33 /= sz;

        Rotation = Quaternion::CreateFromRotationMatrix(rot);
    }

  public:
    Transform &operator=(const Transform &other)
    {
        Position = other.GetPosition();
        Rotation = other.GetRotation();
        Scale = other.GetScale();
        return *this;
    }

    void AddPosition(const Vector3 &InDeltaPosition) { Position += InDeltaPosition; }
    void AddYawRotation(float degree);
    void AddRollRotation(float degree);
    void AddPitchRotation(float degree);

    inline void SetPosition(const Vector3 &InPosition) { Position = InPosition; }
    inline void SetRotation(const Quaternion &InQuaternion) { Rotation = InQuaternion; }
    inline void SetScale(const Vector3 &InScale) { Scale = InScale; }

    inline Vector3    GetPosition() const { return Position; }
    inline Quaternion GetRotation() const { return Rotation; }
    inline Vector3    GetScale() const { return Scale; }

    inline Vector3 GetXAxis() const { return Rotation.ToEuler() * Vector3::UnitY; }
    inline Vector3 GetYAxis() const { return Rotation.ToEuler() * Vector3::UnitX; }
    inline Vector3 GetZAxis() const { return Rotation.ToEuler() * Vector3::UnitZ; }
    
    Vector3 GetForward() const;
    Matrix  GetMatrix() const;

    Transform LocalToWorld(const Transform &inParentWorldTransform) const;

  private:
    Vector3    Position = Vector3::Zero;
    Quaternion Rotation = Quaternion::Identity;
    Vector3    Scale = Vector3::One;
};
