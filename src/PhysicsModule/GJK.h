#pragma once

#include "ColliderBase.h"

bool GJK_DoesIntersect(const Collider* pA, const Collider* pB);
//bool GJK_DoesIntersect();
void GJK_ClosestPoints();