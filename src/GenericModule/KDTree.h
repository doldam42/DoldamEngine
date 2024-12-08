#pragma once

struct KDNode;
struct BoundEdge;

class KDTree
{
  private:
    int       m_maxPrimitive;
    KDNode   *m_pRoot = nullptr;
    Bounds    m_bounds;

  private:
    KDNode *BuildTree(BoundEdge *edges, void **pObjectArray, int objectCount, int depth);
    void    Cleanup();
  public:
    void Initialize(Bounds *boundArray, void **objectArray, int objectCount);
    KDTree(){}
    ~KDTree();
};