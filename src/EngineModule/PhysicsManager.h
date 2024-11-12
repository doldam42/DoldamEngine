#pragma once

struct COLLISION_TEST_RESULT
{

};

class PhysicsManager
{
  public:
    static void CollisionAndResolve(SORT_LINK *pGameObjLink, const UINT objectCount);
};
