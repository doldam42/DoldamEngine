#pragma once

#include "RigidBody.h"

struct CollisionPair
{
    int a;
    int b;

    bool operator==(const CollisionPair &rhs) const
    {
        return (((a == rhs.a) && (b == rhs.b)) || ((a == rhs.b) && (b == rhs.a)));
    }
    bool operator!=(const CollisionPair &rhs) const { return !(*this == rhs); }
};

void BroadPhase(const RigidBody *const *bodies, const int num, std::vector<CollisionPair> &finalPairs,
                const float dt_sec);