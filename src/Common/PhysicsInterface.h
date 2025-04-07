#pragma once

#ifdef PHYSICSMODULE_EXPORTS
#define PHYSICSMODULE_API __declspec(dllexport)
#else
#define PHYSICSMODULE_API __declspec(dllimport)
#endif

#include <combaseapi.h>

#include "../MathModule/MathHeaders.h"
#include "../GenericModule/GenericHeaders.h"

interface IGameObject;
interface ICollider;
interface IRigidBody;
interface ICharacterBody 
{ 
    virtual void Update(IGameObject * pObj) = 0;

    virtual void SetJumpSpeed(float speed) = 0;

    virtual void Move(const Vector3& dir) = 0;
    virtual void Jump() = 0;
    virtual BOOL OnGround() = 0; 
};

interface IPhysicsManager : public IUnknown
{
    virtual BOOL Initialize() = 0;

    virtual ICollider *CreateSphereCollider(const float radius) = 0;
    virtual ICollider *CreateBoxCollider(const Vector3 &halfExtents) = 0;
    virtual ICollider *CreateCapsuleCollider(const float radius, const float height) = 0;
    /*virtual ICollider *CreateEllipseCollider(Vector3 center, float majorRadius,
                                             float minorRadius) = 0;
    virtual ICollider *CreateConvexCollider(const Vector3 *points, const int numPoints) = 0;*/

    virtual IRigidBody *CreateRigidBody(IGameObject * pObj, ICollider * pCollider, float mass, float elasticity,
                                        float friction, BOOL useGravity = TRUE) = 0;
    virtual ICharacterBody *CreateCharacterBody(const Vector3& startPosition, const float radius, const float height) = 0;
    
    virtual void DeleteCollider(ICollider * pDel) = 0;
    virtual void DeleteRigidBody(IRigidBody* pDel) = 0;
    
    virtual void BuildScene() = 0;

    virtual void BeginCollision(float dt) = 0;
    virtual BOOL CollisionTestAll(float dt) = 0;
    virtual void EndCollision() = 0;

    virtual BOOL Raycast(const Ray &ray, float *tHit, IGameObject *pHitted) = 0;
};