#pragma once

#ifdef PHYSICSMODULE_EXPORTS
#define PHYSICSMODULE_API __declspec(dllexport)
#else
#define PHYSICSMODULE_API __declspec(dllimport)
#endif

#include <combaseapi.h>

#include "../GenericModule/GenericHeaders.h"
#include "../MathModule/MathHeaders.h"

enum COLLIDER_TYPE
{
    COLLIDER_TYPE_SPHERE = 0,
    COLLIDER_TYPE_BOX,
    COLLIDER_TYPE_CAPSULE,
    COLLIDER_TYPE_ELLIPSE,
    COLLIDER_TYPE_CONVEX,
};

interface IGameObject;

interface ICollider
{
};

interface IRigidBody
{
    virtual void    Update(IGameObject * pObj) = 0;
    virtual Vector3 GetVelocity() const = 0;
    virtual void    ApplyImpulseLinear(const Vector3 &impulse) = 0;
    virtual void    ApplyImpulseAngular(const Vector3 &impulse) = 0;
    virtual BOOL    IsDynamic() = 0;

    virtual void SetActive(BOOL isActive) = 0;

    virtual void Reset() = 0;

    virtual void SetPosition(const Vector3 &pos) = 0;
    virtual void SetRotation(const Quaternion &q) = 0;

    virtual void SetUserPtr(void *ptr) = 0;
    virtual void *GetUserPtr() = 0;
};

interface ICharacterBody
{
    virtual void Update(IGameObject * pObj) = 0;

    virtual void SetJumpSpeed(float speed) = 0;

    virtual void Move(const Vector3 &dir) = 0;
    virtual void Jump() = 0;
    virtual BOOL OnGround() = 0;
    virtual BOOL CanJump() = 0;
};

interface IHeightFieldTerrainCollider{};

interface IPhysicsManager : public IUnknown
{
    virtual BOOL Initialize() = 0;

    virtual ICollider *CreateSphereCollider(const float radius) = 0;
    virtual ICollider *CreateBoxCollider(const Vector3 &halfExtents) = 0;
    virtual ICollider *CreateCapsuleCollider(const float radius, const float height) = 0;
    virtual ICollider *CreateConvexCollider(const Vector3 *points, const int numPoints) = 0;

    virtual IRigidBody     *CreateRigidBody(ICollider * pCollider, const Vector3& pos, float mass, float elasticity,
                                            float friction, BOOL useGravity = TRUE) = 0;
    virtual ICharacterBody *CreateCharacterBody(const Vector3 &startPosition, const float radius,
                                                const float height) = 0;

    virtual IHeightFieldTerrainCollider *CreateHeightFieldTerrain(const BYTE *pImage, const UINT imgWidth,
                                                          const UINT imgHeight, const Vector3 &scale,
                                                          const float minHeight, const float maxHeight) = 0;

    virtual void DeleteCollider(ICollider * pDel) = 0;
    virtual void DeleteRigidBody(IRigidBody * pDel) = 0;

    virtual void BuildScene() = 0;

    virtual void BeginCollision(float dt) = 0;
    virtual BOOL CollisionTestAll(float dt) = 0;
    virtual void EndCollision() = 0;

    virtual BOOL Raycast(const Ray &ray, float *tHit, IRigidBody **ppHitted) = 0;
};