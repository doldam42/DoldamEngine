#include "pch.h"

#include "../MathModule/MathHeaders.h"

#include "KDTree.h"

struct KDNode
{
    enum Axis : char
    {
        AxisX,
        AxisY,
        AxisZ,
    };
    struct Interior
    {
        Axis    axis;
        float   split;
        KDNode *left;
        KDNode *right;
    };
    struct Leaf
    {
        std::vector<void *> items;
    };
    const static int DIMENSION = 3;

    bool isLeaf;
    union {
        Interior interior;
        Leaf     leaf;
    };
    KDNode() {}
    void InitInterior(Axis axis, float split);
    void InitLeaf(void **objects, BoundEdge *edges, int primitiveCount);
};

enum class EdgeType
{
    Start,
    End
};
struct BoundEdge
{
    float    point[3];
    int      primNum;
    EdgeType type;
    BoundEdge() {}
    BoundEdge(float p[3], int primNum, bool starting) : primNum(primNum)
    {
        memcpy(point, p, sizeof(point));
        type = starting ? EdgeType::Start : EdgeType::End;
    }
};

void KDNode::InitInterior(Axis axis, float split)
{
    this->isLeaf = false;
    this->interior.axis = axis;
    this->interior.split = split;
}

void KDNode::InitLeaf(void** objects, BoundEdge* edges, int primitiveCount)
{
    this->isLeaf = true;
    this->leaf.items = std::vector<void *>(primitiveCount);

    for (int i = 0; i < primitiveCount; i++)
    {
        int index = edges[i].primNum;
        this->leaf.items[i] = objects[index];
    }
}

KDNode *KDTree::BuildTree(BoundEdge *edges, void **pObjectArray, int objectCount, int depth)
{
    // Initialize leaf node if termination criteria met
    if (objectCount == 0)
        return nullptr;
    if (depth == 0)
    {
        KDNode *pNode = new KDNode;
        pNode->InitLeaf(pObjectArray, edges, objectCount);
        return pNode;
    }

    // 현재 분할 축 계산
    int axis = depth % KDNode::DIMENSION;

    // 점들을 분할 축에 따라 정렬
    std::sort(&edges[0], &edges[2 * objectCount], [axis](const BoundEdge &e0, const BoundEdge &e1) -> bool {
        if (e0.point[axis] == e1.point[axis])
            return (int)e0.type < (int)e1.type;
        else
            return e0.point[axis] < e1.point[axis];
    });

    // 중간 점 선택 (균형 이진 트리)
    size_t  mid = objectCount;
    KDNode *pNode = new KDNode;
    pNode->InitInterior((KDNode::Axis)axis, edges[mid].point[axis]);

    // 좌/우 서브트리 생성
    pNode->interior.left = BuildTree(edges, pObjectArray, mid / 2, depth - 1);
    pNode->interior.right = BuildTree(edges + mid + 1, pObjectArray, mid / 2, depth - 1);

    return pNode;
}

void KDTree::Cleanup() {}

void KDTree::Initialize(Bounds *boundArray, void **objectArray, int objectCount)
{
    // bound 계산
    std::vector<Bounds> bounds(objectCount);
    for (int i = 0; i < objectCount; i++)
    {
        Bounds b = boundArray[i];
        m_bounds.Expand(b);
        bounds[i] = b;
    }

    // Bounding Edge (min/max) 초기화
    BoundEdge *pEdges;
    pEdges = new BoundEdge[2 * objectCount];
    for (int i = 0; i < objectCount; i++)
    {
        pEdges[2 * i] = BoundEdge((float *)&bounds[i].mins, i, true);
        pEdges[2 * i + 1] = BoundEdge((float *)&bounds[i].maxs, i, false);
    }

    // 최대 깊이 계산
    int maxDepth = std::round(8 + 1.3f * log2f((float)objectCount));
    
    // 트리 생성
    m_pRoot = BuildTree(pEdges, objectArray, objectCount, maxDepth);

    // 리소스 해제
    if (pEdges)
    {
        delete[] pEdges;
        pEdges = nullptr;
    }
}

KDTree::~KDTree() 
{ Cleanup(); }
