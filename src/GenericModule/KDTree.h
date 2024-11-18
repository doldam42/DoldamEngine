#pragma once

struct KDNode
{
    static const UINT DIMENSION = 3;

    float point[DIMENSION];

    KDNode *pLeft;
    KDNode *pRight;
    void   *pItem;

    KDNode(void *item, const float *pArray) : pLeft(nullptr), pRight(nullptr)
    {
        pItem = item;
        memcpy(point, pArray, sizeof(point));
    }
};

struct KDTree
{
  private:
    KDNode *root;

    KDNode *InsertNodeRecursive(KDNode *pNode, float *point, int depth, void* pItem);

    KDNode *FindMin(KDNode *pNode, int axis, int depth);

    KDNode *DeleteNodeRecursive(KDNode *pNode, float *point, int depth, void *pItem);

    float distance(const float *a, const float *b);

    KDNode *BuildTree(float **points, int pointCount, int depth);

    void NearestNeighborSearch(KDNode *pNode, const float pTarget[3], int depth, KDNode **pOutNearest,
                               float *pOutNearestDist);

  public:
    void Insert(void* pItem, float *point);
    void Delete(void* pItem, float *point);
};
