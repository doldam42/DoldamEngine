#pragma once

#include "PhysicsComponent.h"

struct Contact
{
    Vector3 worldSpaceA;
    Vector3 worldSpaceB;
    Vector3 localSpaceA;
    Vector3 localSpaceB;

    Vector3 normal;
    float   separationDistance;
    float   timeOfImpact;
    
    PhysicsComponent *pA;
    PhysicsComponent *pB;
};

void ResolveContact(Contact& contact);