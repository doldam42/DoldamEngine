#pragma once

struct Collider;
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

    void SortBodiesBounds(const Collider *const *bodies, const int num, PsuedoBody *sortedArray, const float dt_sec);

  public:
    BOOL Initialize(const UINT maxBodyCount, Vector3 axisSAP);

    void Build(const Collider *const *bodies, const int num, const float dt_sec);

    // return num candidate
    UINT QueryCollisionPairs(CollisionPair *pCollisionPairs, UINT maxCollision);

    void Reset();
};
