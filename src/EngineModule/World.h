#pragma once

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

    World(){}
    ~World();
};
