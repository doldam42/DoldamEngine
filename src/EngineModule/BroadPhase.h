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

//void BroadPhase(const RigidBody *const *bodies, const int num, std::vector<CollisionPair> &finalPairs,
//                const float dt_sec);

struct PsuedoBody
{
    int   id;
    float value;
    bool  isMin;
};

class BroadPhase
{
  private:
    PsuedoBody *m_pPsudoBodies = nullptr;

    UINT m_bodyCount;
    UINT m_maxBodyCount;

    Vector3 m_axisSAP;

    void SortBodiesBounds(const RigidBody *const *bodies, const int num, PsuedoBody *sortedArray, const float dt_sec);
    void BuildPairs(std::vector<CollisionPair> &collisionPairs, const PsuedoBody *sortedBodies, const int num);

  public:
    BOOL Initialize(const UINT maxBodyCount, Vector3 axisSAP);

    void Build(const RigidBody *const *bodies, const int num, const float dt_sec);

    // return num candidate
    UINT QueryCollisionPairs(CollisionPair* pCollisionPairs, UINT maxCollision);

    // return num candidate
    UINT QueryIntersectRay(const Ray &ray, int *bodyIDs, UINT maxCollision);

    void Reset();
};
