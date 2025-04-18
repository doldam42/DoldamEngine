#pragma once

struct Collider;
struct Contact
{
    Vector3 contactPointAWorldSpace;
    Vector3 contactPointBWorldSpace;

    Vector3 normal;

    float timeOfImpact = 0.0f;

    Collider *pA = nullptr;
    Collider *pB = nullptr;
};