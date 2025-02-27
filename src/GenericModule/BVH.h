#pragma once

struct BVHBuildNode;
struct BVHPrimitiveInfo;
struct MortonPrimitive;
struct LinearBVHNode;
class BVH
{
public:
	enum class SplitMethod { SAH, HLBVH, Middle, EqualCounts };
private:
	const int maxPrimitiveInNode, maxObjectCount;
	const SplitMethod splitMethod;

	std::vector<IBoundedObject*> primitives;
	LinearBVHNode* nodes = nullptr;

	BVHBuildNode* BuildRecursive
	(
		std::vector<BVHPrimitiveInfo>& primitiveInfo,
		int start, int end, int* totalNodes,
		std::vector<IBoundedObject*>& orderedPrims
	);

	BVHBuildNode* BuildHLBVH(std::vector<BVHPrimitiveInfo>& primitiveInfo,
		int* totalNodes,
		std::vector<IBoundedObject*>& orderedPrims) const;

	BVHBuildNode* emitLBVH(BVHBuildNode*& buildNodes,
		const std::vector<BVHPrimitiveInfo>& primitiveInfo,
		MortonPrimitive* mortonPrims, int nPrimitives, int* totalNodes,
		std::vector<IBoundedObject*>& orderedPrims,
		std::atomic<int>* orderedPrimsOffset, int bitIndex) const;

	BVHBuildNode* buildUpperSAH(std::vector<BVHBuildNode*>& treeletRoots,
		int start, int end, int* totalNodes) const;

	int flattenBVHTree(BVHBuildNode* node, int* offset);


public:

	Bounds GetBounds() const;
	bool IntersectP(const Ray& ray, float* pOutHitt, IBoundedObject** pHitted) const;
	bool Intersect(const Bounds& b) const;

	BOOL InsertObject(IBoundedObject *pObj);

	void Build();

	BVH(int maxObjectCount, int maxPrimsInNode = 1, SplitMethod splitMethod = SplitMethod::HLBVH);
	~BVH();
};
