#pragma once

#include "PhysicsComponent.h"

struct Contact
{
    Vector3 contactPointA;
    Vector3 contactPointB;

    Vector3 normal;
    
    PhysicsComponent *pA;
    PhysicsComponent *pB;
};

void ResolveContact(Contact &contact);
