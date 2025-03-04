#pragma once
struct Ray
{
    Vector3 position;
    Vector3 direction;
    float   tmax;

    Ray() : position(0.0f), direction(0.0f), tmax(FLT_MAX) {}
    Ray(const Vector3 &pos, const Vector3 &dir) : position(pos), direction(dir), tmax(FLT_MAX) {}
    Ray(const Vector3 &pos, const Vector3 &dir, float tmax_) : position(pos), direction(dir), tmax(tmax_) {}
};
