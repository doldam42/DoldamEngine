#pragma once

#include "Contact.h"

BOOL Intersect(PhysicsComponent *pA, PhysicsComponent *pB, const float dt, Contact *pOutContact);