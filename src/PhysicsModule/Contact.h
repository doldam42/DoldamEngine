#pragma once

#include "RigidBody.h"

struct Contact
{
    Vector3 contactPoint;
    Vector3 contactNormal;
    float   penetration;
};

void ResolveContact(Contact &contact);
