
#pragma once

// KDTree Declarations
struct KdAccelNode;
struct BoundEdge;
class KDTree
{
  public:
    // KDTree Public Methods
    KDTree(int maxObjectCount, int isectCost = 80, int traversalCost = 1, float emptyBonus = 0.5, int maxPrims = 1,
           int maxDepth = -1);
    Bounds WorldBound() const { return bounds; }
    ~KDTree();
    // bool Intersect(const Ray& ray, SurfaceInteraction* isect) const;
    bool IntersectP(const Ray &ray, float *pOutHitt, IBoundedObject **pHitted) const;
    bool Intersect(const Bounds &b) const;

    BOOL InsertObject(IBoundedObject *pObj);

    void Build();

  private:
    // KDTree Private Methods
    void buildTree(int nodeNum, const Bounds &bounds, const std::vector<Bounds> &primBounds, int *primNums, int nprims,
                   int depth, BoundEdge *edges[3], int *prims0, int *prims1, int badRefines = 0);

    // KDTree Private Data
    const int   isectCost, traversalCost, maxPrims, maxObjectCount;
    const float emptyBonus;

    int maxDepth;

    std::vector<IBoundedObject *> primitives;
    std::vector<int>              primitiveIndices;
    KdAccelNode                  *nodes;
    int                           nAllocedNodes, nextFreeNode;
    Bounds                        bounds;
};

struct KdToDo
{
    const KdAccelNode *node;
    float              tMin, tMax;
};
