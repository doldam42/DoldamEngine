#include "pch.h"

#include <vector>

#include "BVH.h"

using namespace std;
struct BVHPrimitiveInfo
{
    BVHPrimitiveInfo() {}
    BVHPrimitiveInfo(size_t primitiveNumber, const Bounds &bounds)
        : primitiveNumber(primitiveNumber), bounds(bounds), centroid(bounds.Center())
    {
    }
    size_t  primitiveNumber;
    Bounds  bounds;
    Vector3 centroid;
};

struct BVHBuildNode
{
    // BVHBuildNode Public Methods
    void InitLeaf(int first, int n, const Bounds &b)
    {
        firstPrimOffset = first;
        nPrimitives = n;
        bounds = b;
        children[0] = children[1] = nullptr;
    }
    void InitInterior(int axis, BVHBuildNode *c0, BVHBuildNode *c1)
    {
        children[0] = c0;
        children[1] = c1;
        bounds.Clear();
        bounds.Expand(c0->bounds);
        bounds.Expand(c1->bounds);
        splitAxis = axis;
        nPrimitives = 0;
    }
    Bounds        bounds;
    BVHBuildNode *children[2];
    int           splitAxis, firstPrimOffset, nPrimitives;
};

struct MortonPrimitive
{
    int      primitiveIndex;
    uint32_t mortonCode;
};

struct LBVHTreelet
{
    int           startIndex, nPrimitives;
    BVHBuildNode *buildNodes;
};

struct LinearBVHNode
{
    Bounds bounds;
    union {
        int primitivesOffset;  // leaf
        int secondChildOffset; // interior
    };
    uint16_t nPrimitives; // 0 -> interior node
    uint8_t  axis;        // interior node: xyz
    uint8_t  pad[1];      // ensure 32 byte total size
};

struct BucketInfo
{
    int    count = 0;
    Bounds bounds;
};

inline uint32_t LeftShift3(uint32_t x)
{
    if (x == (1 << 10))
        --x;

    x = (x | (x << 16)) & 0x30000ff;
    x = (x | (x << 8)) & 0x300f00f;
    x = (x | (x << 4)) & 0x30c30c3;
    x = (x | (x << 2)) & 0x9249249;

    return x;
}

inline uint32_t EncodeMorton3(const Vector3 &v)
{
    return (LeftShift3(v.z) << 2) | (LeftShift3(v.y) << 1) | LeftShift3(v.x);
}

static void RadixSort(vector<MortonPrimitive> *v)
{
    vector<MortonPrimitive> tmpVector(v->size());
    constexpr int           bitsPerPass = 6;
    constexpr int           nBits = 30;
    static_assert((nBits % bitsPerPass) == 0, "Radix sort bitsPerPass must evenly divide nBits");
    constexpr int nPasses = nBits / bitsPerPass;

    for (int pass = 0; pass < nPasses; ++pass)
    {
        // Perform one pass of radix sort, sorting _bitsPerPass_ bits
        int lowBit = pass * bitsPerPass;

        // Set in and out vector pointers for radix sort pass
        std::vector<MortonPrimitive> &in = (pass & 1) ? tmpVector : *v;
        std::vector<MortonPrimitive> &out = (pass & 1) ? *v : tmpVector;

        // Count number of zero bits in array for current radix sort bit
        constexpr int nBuckets = 1 << bitsPerPass;
        int           bucketCount[nBuckets] = {0};
        constexpr int bitMask = (1 << bitsPerPass) - 1;
        for (const MortonPrimitive &mp : in)
        {
            int bucket = (mp.mortonCode >> lowBit) & bitMask;
            ++bucketCount[bucket];
        }

        // Compute starting index in output array for each bucket
        int outIndex[nBuckets];
        outIndex[0] = 0;
        for (int i = 1; i < nBuckets; ++i)
            outIndex[i] = outIndex[i - 1] + bucketCount[i - 1];

        // Store sorted values in output array
        for (const MortonPrimitive &mp : in)
        {
            int bucket = (mp.mortonCode >> lowBit) & bitMask;
            out[outIndex[bucket]++] = mp;
        }
    }
    // Copy final result from _tempVector_, if needed
    if (nPasses & 1)
        std::swap(*v, tmpVector);
}

BVHBuildNode *BVH::BuildRecursive(std::vector<BVHPrimitiveInfo> &primitiveInfo, int start, int end, int *totalNodes,
                                  std::vector<IBoundedObject *> &orderedPrims)
{
    BVHBuildNode *node = new BVHBuildNode;
    (*totalNodes)++;
    // Compute bounds of all primitives in BVH node
    Bounds bounds;
    for (int i = start; i < end; ++i)
        bounds.Expand(primitiveInfo[i].bounds);
    int nPrimitives = end - start;
    if (nPrimitives == 1)
    {
        // Create leaf _BVHBuildNode_
        int firstPrimOffset = orderedPrims.size();
        for (int i = start; i < end; ++i)
        {
            int primNum = primitiveInfo[i].primitiveNumber;
            orderedPrims.push_back(primitives[primNum]);
        }
        node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
        return node;
    }
    else
    {
        // Compute bound of primitive centroids, choose split dimension _dim_
        Bounds centroidBounds;
        for (int i = start; i < end; ++i)
            centroidBounds.Expand(primitiveInfo[i].centroid);
        int dim = centroidBounds.MaximumExtent();
        // Partition primitives into two sets and build children
        int mid = (start + end) / 2;

        if (centroidBounds.maxs[dim] == centroidBounds.mins[dim])
        {
            // Create leaf _BVHBuildNode_
            int firstPrimOffset = orderedPrims.size();
            for (int i = start; i < end; ++i)
            {
                int primNum = primitiveInfo[i].primitiveNumber;
                orderedPrims.push_back(primitives[primNum]);
            }
            node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
            return node;
        }
        else
        {
            // Partition primitives based on _splitMethod_
            switch (splitMethod)
            {
            case SplitMethod::Middle: {
                // Partition primitives through node's midpoint
                float             pmid = (centroidBounds.mins[dim] + centroidBounds.maxs[dim]) / 2;
                BVHPrimitiveInfo *midPtr =
                    std::partition(&primitiveInfo[start], &primitiveInfo[end - 1] + 1,
                                   [dim, pmid](const BVHPrimitiveInfo &pi) { return pi.centroid[dim] < pmid; });
                mid = midPtr - &primitiveInfo[0];
                // For lots of prims with large overlapping bounding boxes, this
                // may fail to partition; in that case don't break and fall
                // through
                // to EqualCounts.
                if (mid != start && mid != end)
                    break;
            }
            case SplitMethod::EqualCounts: {
                // Partition primitives into equally-sized subsets
                mid = (start + end) / 2;
                std::nth_element(&primitiveInfo[start], &primitiveInfo[mid], &primitiveInfo[end - 1] + 1,
                                 [dim](const BVHPrimitiveInfo &a, const BVHPrimitiveInfo &b) {
                                     return a.centroid[dim] < b.centroid[dim];
                                 });
                break;
            }
            case SplitMethod::SAH:
            default: {
                // Partition primitives using approximate SAH
                if (nPrimitives <= 2)
                {
                    // Partition primitives into equally-sized subsets
                    mid = (start + end) / 2;
                    std::nth_element(&primitiveInfo[start], &primitiveInfo[mid], &primitiveInfo[end - 1] + 1,
                                     [dim](const BVHPrimitiveInfo &a, const BVHPrimitiveInfo &b) {
                                         return a.centroid[dim] < b.centroid[dim];
                                     });
                }
                else
                {
                    // Allocate _BucketInfo_ for SAH partition buckets
                    constexpr int nBuckets = 12;
                    BucketInfo    buckets[nBuckets];

                    // Initialize _BucketInfo_ for SAH partition buckets
                    for (int i = start; i < end; ++i)
                    {
                        int b = nBuckets * centroidBounds.Offset(primitiveInfo[i].centroid)[dim];
                        if (b == nBuckets)
                            b = nBuckets - 1;
                        buckets[b].count++;
                        buckets[b].bounds.Expand(primitiveInfo[i].bounds);
                    }

                    // Compute costs for splitting after each bucket
                    float cost[nBuckets - 1];
                    for (int i = 0; i < nBuckets - 1; ++i)
                    {
                        Bounds b0, b1;
                        int    count0 = 0, count1 = 0;
                        for (int j = 0; j <= i; ++j)
                        {
                            b0.Expand(buckets[j].bounds);
                            count0 += buckets[j].count;
                        }
                        for (int j = i + 1; j < nBuckets; ++j)
                        {
                            b1.Expand(buckets[j].bounds);
                            count1 += buckets[j].count;
                        }
                        cost[i] = 1 + (count0 * b0.SurfaceArea() + count1 * b1.SurfaceArea()) / bounds.SurfaceArea();
                    }

                    // Find bucket to split at that minimizes SAH metric
                    float minCost = cost[0];
                    int   minCostSplitBucket = 0;
                    for (int i = 1; i < nBuckets - 1; ++i)
                    {
                        if (cost[i] < minCost)
                        {
                            minCost = cost[i];
                            minCostSplitBucket = i;
                        }
                    }

                    // Either create leaf or split primitives at selected SAH
                    // bucket
                    float leafCost = nPrimitives;
                    if (nPrimitives > maxPrimitiveInNode || minCost < leafCost)
                    {
                        BVHPrimitiveInfo *pmid = std::partition(
                            &primitiveInfo[start], &primitiveInfo[end - 1] + 1, [=](const BVHPrimitiveInfo &pi) {
                                int b = nBuckets * centroidBounds.Offset(pi.centroid)[dim];
                                if (b == nBuckets)
                                    b = nBuckets - 1;
                                return b <= minCostSplitBucket;
                            });
                        mid = pmid - &primitiveInfo[0];
                    }
                    else
                    {
                        // Create leaf _BVHBuildNode_
                        int firstPrimOffset = orderedPrims.size();
                        for (int i = start; i < end; ++i)
                        {
                            int primNum = primitiveInfo[i].primitiveNumber;
                            orderedPrims.push_back(primitives[primNum]);
                        }
                        node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
                        return node;
                    }
                }
                break;
            }
            }
            node->InitInterior(dim, BuildRecursive(primitiveInfo, start, mid, totalNodes, orderedPrims),
                               BuildRecursive(primitiveInfo, mid, end, totalNodes, orderedPrims));
        }
    }
    return node;
}

BVHBuildNode *BVH::BuildHLBVH(std::vector<BVHPrimitiveInfo> &primitiveInfo, int *totalNodes,
                              std::vector<IBoundedObject *> &orderedPrims) const
{
    // Compute bounding box of all primitive centroids
    Bounds bounds;
    for (const BVHPrimitiveInfo &pi : primitiveInfo)
        bounds.Expand(pi.centroid);

    // Compute Morton indices of primitives
    std::vector<MortonPrimitive> mortonPrims(primitiveInfo.size());

    for (int i = 0; i < primitiveInfo.size(); i++)
    {
        // Initialize _mortonPrims[i]_ for _i_th primitive
        constexpr int mortonBits = 10;
        constexpr int mortonScale = 1 << mortonBits;
        mortonPrims[i].primitiveIndex = primitiveInfo[i].primitiveNumber;
        Vector3 centroidOffset = bounds.Offset(primitiveInfo[i].centroid);
        mortonPrims[i].mortonCode = EncodeMorton3(centroidOffset * mortonScale);
    }

    // Radix sort primitive Morton indices
    RadixSort(&mortonPrims);

    // Create LBVH treelets at bottom of BVH

    // Find intervals of primitives for each treelet
    std::vector<LBVHTreelet> treeletsToBuild;
    for (int start = 0, end = 1; end <= (int)mortonPrims.size(); ++end)
    {
        uint32_t mask = 0x3ffc0000;
        if (end == (int)mortonPrims.size() ||
            ((mortonPrims[start].mortonCode & mask) != (mortonPrims[end].mortonCode & mask)))
        {
            // Add entry to _treeletsToBuild_ for this treelet
            int           nPrimitives = end - start;
            int           maxBVHNodes = 2 * nPrimitives;
            BVHBuildNode *nodes = new BVHBuildNode[maxBVHNodes];
            treeletsToBuild.push_back({start, nPrimitives, nodes});
            start = end;
        }
    }

    // Create LBVHs for treelets in parallel
    std::atomic<int> atomicTotal(0), orderedPrimsOffset(0);
    orderedPrims.resize(primitives.size());
    for (int i = 0; i < treeletsToBuild.size(); i++)
    {
        // Generate _i_th LBVH treelet
        int          nodesCreated = 0;
        const int    firstBitIndex = 29 - 12;
        LBVHTreelet &tr = treeletsToBuild[i];
        tr.buildNodes = emitLBVH(tr.buildNodes, primitiveInfo, &mortonPrims[tr.startIndex], tr.nPrimitives,
                                 &nodesCreated, orderedPrims, &orderedPrimsOffset, firstBitIndex);
        atomicTotal += nodesCreated;
    }
    *totalNodes = atomicTotal;

    // Create and return SAH BVH from LBVH treelets
    std::vector<BVHBuildNode *> finishedTreelets;
    finishedTreelets.reserve(treeletsToBuild.size());
    for (LBVHTreelet &treelet : treeletsToBuild)
        finishedTreelets.push_back(treelet.buildNodes);
    return buildUpperSAH(finishedTreelets, 0, finishedTreelets.size(), totalNodes);
}

BVHBuildNode *BVH::emitLBVH(BVHBuildNode *&buildNodes, const std::vector<BVHPrimitiveInfo> &primitiveInfo,
                            MortonPrimitive *mortonPrims, int nPrimitives, int *totalNodes,
                            std::vector<IBoundedObject *> &orderedPrims, std::atomic<int> *orderedPrimsOffset,
                            int bitIndex) const
{
    if (bitIndex == -1 || nPrimitives < maxPrimitiveInNode)
    {
        // Create and return leaf node of LBVH treelet
        (*totalNodes)++;
        BVHBuildNode *node = buildNodes++;
        Bounds        bounds;
        int           firstPrimOffset = orderedPrimsOffset->fetch_add(nPrimitives);
        for (int i = 0; i < nPrimitives; ++i)
        {
            int primitiveIndex = mortonPrims[i].primitiveIndex;
            orderedPrims[firstPrimOffset + i] = primitives[primitiveIndex];
            bounds.Expand(primitiveInfo[primitiveIndex].bounds);
        }
        node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
        return node;
    }
    else
    {
        int mask = 1 << bitIndex;
        // Advance to next subtree level if there's no LBVH split for this bit
        if ((mortonPrims[0].mortonCode & mask) == (mortonPrims[nPrimitives - 1].mortonCode & mask))
            return emitLBVH(buildNodes, primitiveInfo, mortonPrims, nPrimitives, totalNodes, orderedPrims,
                            orderedPrimsOffset, bitIndex - 1);

        // Find LBVH split point for this dimension
        int searchStart = 0, searchEnd = nPrimitives - 1;
        while (searchStart + 1 != searchEnd)
        {
            int mid = (searchStart + searchEnd) / 2;
            if ((mortonPrims[searchStart].mortonCode & mask) == (mortonPrims[mid].mortonCode & mask))
                searchStart = mid;
            else
            {
                searchEnd = mid;
            }
        }
        int splitOffset = searchEnd;

        // Create and return interior LBVH node
        (*totalNodes)++;
        BVHBuildNode *node = buildNodes++;
        BVHBuildNode *lbvh[2] = {emitLBVH(buildNodes, primitiveInfo, mortonPrims, splitOffset, totalNodes, orderedPrims,
                                          orderedPrimsOffset, bitIndex - 1),
                                 emitLBVH(buildNodes, primitiveInfo, &mortonPrims[splitOffset],
                                          nPrimitives - splitOffset, totalNodes, orderedPrims, orderedPrimsOffset,
                                          bitIndex - 1)};
        int           axis = bitIndex % 3;
        node->InitInterior(axis, lbvh[0], lbvh[1]);
        return node;
    }
}

BVHBuildNode *BVH::buildUpperSAH(std::vector<BVHBuildNode *> &treeletRoots, int start, int end, int *totalNodes) const
{
    int nNodes = end - start;
    if (nNodes == 1)
        return treeletRoots[start];
    (*totalNodes)++;
    BVHBuildNode *node = new BVHBuildNode;

    // Compute bounds of all nodes under this HLBVH node
    Bounds bounds;
    for (int i = start; i < end; ++i)
        bounds.Expand(treeletRoots[i]->bounds);

    // Compute bound of HLBVH node centroids, choose split dimension _dim_
    Bounds centroidBounds;
    for (int i = start; i < end; ++i)
    {
        Vector3 centroid = (treeletRoots[i]->bounds.mins + treeletRoots[i]->bounds.maxs) * 0.5f;
        centroidBounds.Expand(centroid);
    }
    int dim = centroidBounds.MaximumExtent();

    // Allocate _BucketInfo_ for SAH partition buckets
    constexpr int nBuckets = 12;
    BucketInfo    buckets[nBuckets];

    // Initialize _BucketInfo_ for HLBVH SAH partition buckets
    for (int i = start; i < end; ++i)
    {
        float centroid = (treeletRoots[i]->bounds.mins[dim] + treeletRoots[i]->bounds.maxs[dim]) * 0.5f;
        int   b =
            nBuckets * ((centroid - centroidBounds.mins[dim]) / (centroidBounds.maxs[dim] - centroidBounds.mins[dim]));
        if (b == nBuckets)
            b = nBuckets - 1;
        buckets[b].count++;
        buckets[b].bounds.Expand(treeletRoots[i]->bounds);
    }

    // Compute costs for splitting after each bucket
    float cost[nBuckets - 1];
    for (int i = 0; i < nBuckets - 1; ++i)
    {
        Bounds b0, b1;
        int    count0 = 0, count1 = 0;
        for (int j = 0; j <= i; ++j)
        {
            b0.Expand(buckets[j].bounds);
            count0 += buckets[j].count;
        }
        for (int j = i + 1; j < nBuckets; ++j)
        {
            b1.Expand(buckets[j].bounds);
            count1 += buckets[j].count;
        }
        cost[i] = .125f + (count0 * b0.SurfaceArea() + count1 * b1.SurfaceArea()) / bounds.SurfaceArea();
    }

    // Find bucket to split at that minimizes SAH metric
    float minCost = cost[0];
    int   minCostSplitBucket = 0;
    for (int i = 1; i < nBuckets - 1; ++i)
    {
        if (cost[i] < minCost)
        {
            minCost = cost[i];
            minCostSplitBucket = i;
        }
    }

    // Split nodes and create interior HLBVH SAH node
    BVHBuildNode **pmid =
        std::partition(&treeletRoots[start], &treeletRoots[end - 1] + 1, [=](const BVHBuildNode *node) {
            float centroid = (node->bounds.mins[dim] + node->bounds.maxs[dim]) * 0.5f;
            int   b = nBuckets *
                    ((centroid - centroidBounds.mins[dim]) / (centroidBounds.maxs[dim] - centroidBounds.mins[dim]));
            if (b == nBuckets)
                b = nBuckets - 1;
            return b <= minCostSplitBucket;
        });
    int mid = pmid - &treeletRoots[0];
    node->InitInterior(dim, this->buildUpperSAH(treeletRoots, start, mid, totalNodes),
                       this->buildUpperSAH(treeletRoots, mid, end, totalNodes));
    return node;
}

int BVH::flattenBVHTree(BVHBuildNode *node, int *offset)
{
    LinearBVHNode *linearNode = &nodes[*offset];
    linearNode->bounds = node->bounds;
    int myOffset = (*offset)++;
    if (node->nPrimitives > 0)
    {
        linearNode->primitivesOffset = node->firstPrimOffset;
        linearNode->nPrimitives = node->nPrimitives;
    }
    else
    {
        // Create interior flattened BVH node
        linearNode->axis = node->splitAxis;
        linearNode->nPrimitives = 0;
        flattenBVHTree(node->children[0], offset);
        linearNode->secondChildOffset = flattenBVHTree(node->children[1], offset);
    }
    return myOffset;
}

Bounds BVH::GetBounds() const { return nodes ? nodes[0].bounds : Bounds(); }

bool BVH::IntersectP(const Ray &ray, float *pOutHitt, IBoundedObject **ppHitted) const
{
    if (!nodes)
        return false;

    float closestHit = FLT_MAX;
    IBoundedObject *pHitted = nullptr;

    int     nodesToVisit[64];
    int     toVisitOffset = 0, currentNodeIndex = 0;
    while (true)
    {
        const LinearBVHNode *node = &nodes[currentNodeIndex];
        
        // Process BVH node _node_ for traversal
        if (node->nPrimitives > 0)
        {
            float tmin, tmax;
            for (int i = 0; i < node->nPrimitives; ++i)
            {
                if (primitives[node->primitivesOffset + i]->Intersect(ray, &tmin, &tmax) && tmin < closestHit)
                {
                    closestHit = tmin;
                    pHitted = primitives[node->primitivesOffset + i];
                }
            }
            if (toVisitOffset == 0)
                break;
            currentNodeIndex = nodesToVisit[--toVisitOffset];
        }
        else
        {
            float tmin0, tmin1, tmax0, tmax1;

            const LinearBVHNode *l = &nodes[currentNodeIndex + 1];
            const LinearBVHNode *r = &nodes[node->secondChildOffset];

            bool intersectL = l->bounds.IntersectP(ray, &tmin0, &tmax0);
            bool intersectR = r->bounds.IntersectP(ray, &tmin1, &tmax1);

            if (intersectL && intersectR)
            {
                if (tmin0 < tmin1)
                {
                    nodesToVisit[toVisitOffset++] = node->secondChildOffset;
                    currentNodeIndex = currentNodeIndex + 1;
                }
                else
                {
                    /// second child first
                    nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
                    currentNodeIndex = node->secondChildOffset;
                }
            }
            else if (intersectL)
            {
                currentNodeIndex = currentNodeIndex + 1;
            }
            else if (intersectR)
            {
                currentNodeIndex = node->secondChildOffset;
            }
            else
            {
                if (toVisitOffset == 0)
                    break;
                currentNodeIndex = nodesToVisit[--toVisitOffset];
            }
        }
    }

    if (closestHit < FLT_MAX)
    {
        *pOutHitt = closestHit;
        *ppHitted = pHitted;
        return true;
    }
    
    return false;
}

bool BVH::Intersect(const Bounds &b) const
{
    if (!nodes)
        return false;

    int nodesToVisit[64];
    int toVisitOffset = 0, currentNodeIndex = 0;
    while (true)
    {
        const LinearBVHNode *node = &nodes[currentNodeIndex];
        float                tmin, tmax;
        if (node->bounds.DoesIntersect(b))
        {
            // Process BVH node _node_ for traversal
            if (node->nPrimitives > 0)
            {
                for (int i = 0; i < node->nPrimitives; ++i)
                {
                    if (primitives[node->primitivesOffset + i]->Intersect(b))
                    {
                        return true;
                    }
                }
                if (toVisitOffset == 0)
                    break;
                currentNodeIndex = nodesToVisit[--toVisitOffset];
            }
            else
            {
                nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
                nodesToVisit[toVisitOffset++] = node->secondChildOffset;
                currentNodeIndex = node->secondChildOffset;
            }
        }
        else
        {
            if (toVisitOffset == 0)
                break;
            currentNodeIndex = nodesToVisit[--toVisitOffset];
        }
    }
    return false;
}

BOOL BVH::InsertObject(IBoundedObject *pObj)
{
    if (primitives.size() > maxObjectCount)
    {
#ifdef _DEBUG
        __debugbreak();
#endif
        return FALSE;
    }

    primitives.push_back(pObj);
    return TRUE;
}

void BVH::Build()
{
    if (primitives.empty())
        return;

    std::vector<BVHPrimitiveInfo> primitiveInfo;
    primitiveInfo.reserve(primitives.size());
    for (size_t i = 0; i < primitives.size(); ++i)
    {
        primitiveInfo.emplace_back(i, primitives[i]->GetBounds());
    }

    // Build BVH tree for primitives
    int                           totalNodes = 0;
    std::vector<IBoundedObject *> orderedPrims;
    orderedPrims.reserve(primitives.size());
    BVHBuildNode *root;
    if (splitMethod == SplitMethod::HLBVH)
        root = BuildHLBVH(primitiveInfo, &totalNodes, orderedPrims);
    else
        root = BuildRecursive(primitiveInfo, 0, primitives.size(), &totalNodes, orderedPrims);

    primitives.swap(orderedPrims);
    primitiveInfo.resize(0);

    // Compute representation of depth-first traversal of BVH tree
    nodes = new LinearBVHNode[totalNodes];
    int offset = 0;
    flattenBVHTree(root, &offset);
}

BVH::BVH(int maxObjectCount, int maxPrimsInNode, SplitMethod splitMethod_)
    : maxPrimitiveInNode(min(255, maxPrimsInNode)), splitMethod(splitMethod_), maxObjectCount(maxObjectCount)
{
    primitives.reserve(maxObjectCount);
}

BVH::~BVH()
{
    if (nodes)
    {
        delete[] nodes;
        nodes = nullptr;
    }
}
