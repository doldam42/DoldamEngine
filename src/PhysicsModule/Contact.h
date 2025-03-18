#pragma once

#include "RigidBody.h"

struct Contact
{
    Vector3 contactPointAWorldSpace = Vector3::Zero;
    Vector3 contactPointBWorldSpace = Vector3::Zero;

    Vector3 contactPointALocalSpace = Vector3::Zero;
    Vector3 contactPointBLocalSpace = Vector3::Zero;

    Vector3 normal = Vector3::Zero;

    float timeOfImpact = 0.0f;
    float separationDistance = 0.0f;

    float normalImpulse = 0.0f;
    //float tangentImpulse = 0.0f;
    
    RigidBody *pA = nullptr;
    RigidBody *pB = nullptr;
};

void ResolveContact(Contact &contact);
