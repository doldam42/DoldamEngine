#pragma once
struct Plane
{
    Vector3 normal = Vector3::UnitY;
    float   d = 0.0f;

    inline Plane() = default;
    inline explicit Plane(const Vector3 &InNormal, float InDistance) : normal(InNormal), d(InDistance)
    {
        assert(InNormal.LengthSquared() == 1);
    } // 정규화된 요소로 평면 생성하기
    inline explicit Plane(const Vector3 &InNormal,
                             Vector3        InPlanePoint) // 정규화된 요소로 평면 생성하기
    {
        assert(InNormal.LengthSquared() == 1);
        d = -normal.Dot(InPlanePoint);
    }
    explicit Plane(const Vector3 &InPoint1, const Vector3 &InPoint2, const Vector3 &InPoint3);
    explicit Plane(const Vector4 &InVector4); // 정규화안된 네 개의 요소로 평면 생성하기

    float Distance(const Vector3 &InPoint) const { return normal.Dot(InPoint) + d; }
    bool  IsOutside(const Vector3 &InPoint) const { return Distance(InPoint) > 0.f; }

  private:
    void Normalize();
};
