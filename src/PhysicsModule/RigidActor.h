#pragma once

#include "ColliderBase.h"

struct RigidActor : public IRigidActor
{
    Collider *pCollider;
    BOOL IsDynamic;
    BOOL IsPrevCollide;
    BOOL IsCollide;

    BOOL IsDynamic() override { return IsDynamic; }
    BOOL IsCollisionEnter() override { return (!IsPrevCollide && IsCollide); }
    BOOL IsCollisionStay() override { return (IsPrevCollide && IsCollide); }
    BOOL IsCollisionExit() override { return (IsPrevCollide && !IsCollide); }
};
