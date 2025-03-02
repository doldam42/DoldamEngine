#pragma once

class KDTree;
class GameObject;
class World
{
  private:
    KDTree* m_pTree = nullptr;

  private:
    void Cleanup();
  public:
    void Initialize();
    void BeginCreateWorld(UINT maxObjectCount);
    void InsertObject(GameObject *pObject);
    void EndCreateWorld();

    bool Intersect(const Ray &ray, RayHit* pOutHit);

    World(){}
    ~World();
};
