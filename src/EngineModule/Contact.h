#pragma once

#include "PhysicsComponent.h"

struct Contact
{
    Vector3 contactPointAWorldSpace;
    Vector3 contactPointBWorldSpace;

    Vector3 contactPointALocalSpace;
    Vector3 contactPointBLocalSpace;

    Vector3 normal;

    float timeOfImpact;
    float separationDistance;
    
    PhysicsComponent *pA;
    PhysicsComponent *pB;
};

void ResolveContact(Contact &contact);
