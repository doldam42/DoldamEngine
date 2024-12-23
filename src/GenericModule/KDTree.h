#pragma once

#include "../MathModule/Bounds.h"
struct KDNode;
struct BoundEdge;
class KDTree
{
  private:
    int m_maxObjectCount = 0;
    int m_curObjectCount = 0;
    int m_depth = 0;

    int m_allocatedInteriorNodeCount = 0;
    int m_allocatedLeafNodeCount = 0;
    int m_maxInteriorNodeCount = 0;
    int m_maxLeafNodeCount = 0;

    KDNode     *m_pRoot = nullptr;
    Bounds      m_totalBounds;
    Bounds     *m_pBoundArray = nullptr;
    SORT_LINK **m_pLinkArray = nullptr;

    KDNode *m_pLeafNodes = nullptr;
    KDNode *m_pInteriorNodes = nullptr;

  private:
    KDNode *BuildTreeRecursive(BoundEdge *edges, SORT_LINK **pObjectArray, int objectCount, int depth);

    void InsertDynamicObjectRecursive(KDNode *pNode, const Vector3 &pos, SORT_LINK *pNew, int depth);
    void DeleteDynamicObjectRecursive(KDNode *pNode, const Vector3 &pos, SORT_LINK *pDel, int depth);

    void Cleanup();
    void DebugPrintTreeRecursive(KDNode *pNode);

  public:
    void BeginCreateTree(int maxObjectCount);
    void InsertObject(const Bounds *pBounds, SORT_LINK *pNew);
    void EndCreateTree();

    // Ʈ�� ���� �� Update�ϴ� ��� AABB�� ����ϸ� split�� �����ؾ� �ϴ� ������ �߻�. ���� AABB�� �ƴ� Position��
    // ���
    void InsertDynamicObject(const Vector3 &pos, SORT_LINK *pNew);
    void DeleteDynamicObject(const Vector3 &pos, SORT_LINK *pDel);
    void UpdateDynamicObject(const Vector3 &pos, SORT_LINK *pObj);

    int    GetObjectCount() const { return m_curObjectCount; }
    int    GetDepth() const { return m_depth; }
    Bounds GetTotalBound() const { return m_totalBounds; }

    void DebugPrintTree();

    KDTree() {}
    ~KDTree();
};
