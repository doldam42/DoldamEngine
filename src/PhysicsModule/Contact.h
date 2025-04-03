#pragma once

#include "RigidBody.h"

struct Contact
{
    Vector3 position;
    Vector3 normal;
    float   penetration;
};

void ResolveContact(Contact &contact);
