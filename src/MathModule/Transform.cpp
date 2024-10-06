#include "pch.h"

void Transform::AddYawRotation(float degree)
{
    Vector3 r = Rotation.ToEuler();
    r.y += degree;
    Rotation = Quaternion::CreateFromYawPitchRoll(r);
}

void Transform::AddRollRotation(float degree)
{
    Vector3 r = Rotation.ToEuler();
    r.z += degree;
    Rotation = Quaternion::CreateFromYawPitchRoll(r);
}

void Transform::AddPitchRotation(float degree)
{
    Vector3 r = Rotation.ToEuler();
    r.x += degree;
    Rotation = Quaternion::CreateFromYawPitchRoll(r);
}

Vector3 Transform::GetForward() const
{
    // RollÀº ¹«½Ã
    Vector3 yawPitchRoll = Rotation.ToEuler();
    yawPitchRoll.y += DirectX::XM_PI;
    // float yaw = Rotation.ToEuler().y;
    // yaw += DirectX::XM_PI;
    return Vector3::Transform(Vector3(0.0f, 0.0f, 1.0f), Matrix::CreateFromYawPitchRoll(yawPitchRoll));
}

Matrix Transform::GetMatrix() const
{
    return Matrix::CreateScale(Scale) * Matrix::CreateFromQuaternion(Rotation) * Matrix::CreateTranslation(Position);
}

Transform Transform::LocalToWorld(const Transform &inParentWorldTransform) const
{
    Transform result;
    GetScale() * GetScale();

    result.SetScale(inParentWorldTransform.GetScale() * GetScale());
    result.SetRotation(inParentWorldTransform.GetRotation() * GetRotation());
    Vector3 deltaPos = inParentWorldTransform.GetRotation() * (inParentWorldTransform.GetScale() * GetPosition());
    result.SetPosition(inParentWorldTransform.GetPosition() + deltaPos);

    return result;
}
