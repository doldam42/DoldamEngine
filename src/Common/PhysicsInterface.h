#pragma once

#ifdef PHYSICSMODULE_EXPORTS
#define PHYSICSMODULE_API __declspec(dllexport)
#else
#define PHYSICSMODULE_API __declspec(dllimport)
#endif

#include <combaseapi.h>

#include "../GenericModule/GenericHeaders.h"
#include "../MathModule/MathHeaders.h"
#include "../Common/EngineInterface.h"

enum DP_COLLIDER_TYPE
{
    DP_COLLIDER_TYPE_SPHERE = 0,
    DP_COLLIDER_TYPE_BOX,
    DP_COLLIDER_TYPE_CAPSULE,
    DP_COLLIDER_TYPE_ELLIPSE,
    DP_COLLIDER_TYPE_CONVEX,
};

interface ICollider
{
    virtual DP_COLLIDER_TYPE GetType() const = 0;

    virtual void Update() = 0;

    virtual Vector3 GetCenter() const = 0;
    virtual Vector3 GetWorldCenter() const = 0;
    virtual Bounds  GetBounds() const = 0;
    virtual Bounds  GetWorldBounds() const = 0;

    virtual Matrix InertiaTensor() const = 0;

    virtual BOOL Intersect(ICollider * pOther) const = 0;
    virtual BOOL IntersectRay(const Ray &ray, float *hitt0, float *hitt1) const = 0;
    virtual BOOL Intersect(const Bounds &b) const = 0;

    // Find the point in furthest in direction
    virtual Vector3 Support(const Vector3 dir, const Vector3 pos, const Quaternion orient, const float bias) = 0;
    virtual float   FastestLinearSpeed(const Vector3 angularVelocity, const Vector3 dir) const = 0;
};

interface IRigidBody
{
    virtual Vector3 GetVelocity() const = 0;
    virtual void    ApplyImpulseLinear(const Vector3 &impulse) = 0;
    virtual void    ApplyImpulseAngular(const Vector3 &impulse) = 0;

    virtual BOOL IsFixed() const = 0;
};

interface IPhysicsManager
{
    virtual ICollider *CreateSphereCollider(IGameObject * pObj, const Vector3 &center, const float radius) = 0;
    virtual ICollider *CreateBoxCollider(IGameObject * pObj, const Vector3 &center, const Vector3 &extents) = 0;
    virtual ICollider *CreateEllipseCollider(IGameObject * pObj, Vector3 center, float majorRadius,
                                             float minorRadius) = 0;
    virtual ICollider *CreateConvexCollider(IGameObject * pObj, const Vector3 *points, const int numPoints) = 0;

    virtual IRigidBody *CreateRigidBody(IGameObject * pObj, ICollider * pCollider, float mass, float elasticity,
                                        float friction, BOOL useGravity = TRUE) = 0;
    
    virtual void BuildScene() = 0;

    virtual void BeginCollision(float dt) = 0;
    virtual BOOL CollisionTestAll(float dt) = 0;
    virtual void EndCollision() = 0;

    virtual BOOL Raycast(const Ray &ray, float *tHit, IGameObject *pHitted) = 0;
};