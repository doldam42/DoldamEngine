#include "pch.h"

void Transform::AddYawRotation(float degree)
{
    Vector3 r = Rotation.ToEuler();
    r.x += degree;
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
    r.y += degree;
    Rotation = Quaternion::CreateFromYawPitchRoll(r);
}

Vector3 Transform::GetForward() const
{
    // RollÀº ¹«½Ã
    Vector3 yawPitchRoll = Rotation.ToEuler();
    return Vector3::Transform(Vector3::Forward, Matrix::CreateFromYawPitchRoll(yawPitchRoll));
}

Matrix Transform::GetMatrix() const
{
    return Matrix::CreateScale(Scale) * Matrix::CreateFromQuaternion(Rotation) * Matrix::CreateTranslation(Position);
}

Transform Transform::LocalToWorld(const Transform &inParentWorldTransform) const
{
    Transform result;

    /*result.SetScale(inParentWorldTransform.GetScale() * GetScale());

    result.SetRotation(Quaternion::Concatenate(inParentWorldTransform.GetRotation(), GetRotation()));

    Vector3 deltaPos =
        Vector3::Transform(inParentWorldTransform.GetScale() * GetPosition(), inParentWorldTransform.GetRotation());
    result.SetPosition(inParentWorldTransform.GetPosition() + deltaPos);

    return result;*/
    return Transform(GetMatrix() * inParentWorldTransform.GetMatrix());
}
