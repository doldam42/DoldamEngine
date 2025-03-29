#pragma once

struct BVHBuildNode;
struct BVHPrimitiveInfo;
struct MortonPrimitive;
struct LinearBVHNode;
class BVH
{
  public:
    enum class SplitMethod
    {
        SAH,
        HLBVH,
        Middle,
        EqualCounts
    };
    struct Primitive
    {
        Bounds      bounds;
        void *pObj;
    };

  private:
    const int         maxPrimitiveInNode, maxObjectCount;
    const SplitMethod splitMethod;

    std::vector<Primitive> primitives;
    LinearBVHNode         *nodes = nullptr;

    BVHBuildNode *BuildRecursive(std::vector<BVHPrimitiveInfo> &primitiveInfo, int start, int end, int *totalNodes,
                                 std::vector<Primitive> &orderedPrims);

    BVHBuildNode *BuildHLBVH(std::vector<BVHPrimitiveInfo> &primitiveInfo, int *totalNodes,
                             std::vector<Primitive> &orderedPrims) const;

    BVHBuildNode *emitLBVH(BVHBuildNode *&buildNodes, const std::vector<BVHPrimitiveInfo> &primitiveInfo,
                           MortonPrimitive *mortonPrims, int nPrimitives, int *totalNodes,
                           std::vector<Primitive> &orderedPrims, std::atomic<int> *orderedPrimsOffset,
                           int bitIndex) const;

    BVHBuildNode *buildUpperSAH(std::vector<BVHBuildNode *> &treeletRoots, int start, int end, int *totalNodes) const;

    int flattenBVHTree(BVHBuildNode *node, int *offset);

  public:
    Bounds GetBounds() const;
    bool   IntersectP(const Ray &ray, float *pOutHitt, void **ppHitted) const;
    bool   Intersect(const Bounds &b, void** ppObj) const;

    BOOL InsertObject(const Bounds &b, void *pObj);

    void Build();

    BVH(int maxObjectCount, int maxPrimsInNode = 1, SplitMethod splitMethod = SplitMethod::HLBVH);
    ~BVH();
};
