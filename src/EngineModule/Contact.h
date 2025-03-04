#pragma once

#include "RigidBody.h"

struct Contact
{
    Vector3 contactPointAWorldSpace;
    Vector3 contactPointBWorldSpace;

    Vector3 contactPointALocalSpace;
    Vector3 contactPointBLocalSpace;

    Vector3 normal;

    float timeOfImpact;
    float separationDistance;
    
    RigidBody *pA;
    RigidBody *pB;
};

void ResolveContact(Contact &contact);
