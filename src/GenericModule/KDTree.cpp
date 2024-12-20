#include "pch.h"

#include "KDTree.h"

struct KDNode
{
    const static int DIMENSION = 3;
    enum Axis : char
    {
        AxisX,
        AxisY,
        AxisZ,
    };
    struct LeafItems
    {
        SORT_LINK *pItemLinkHead;
        SORT_LINK *pItemLinkTail;
    };
    struct Child
    {
        KDNode *left;
        KDNode *right;
    };

    bool  isLeaf;
    Axis  axis;
    float split;

    union {
        LeafItems leafItems;
        Child     child;
    };

    void InitLeaf(SORT_LINK **objects, BoundEdge *edges, int itemCount);
    void InitInterior(Axis axis, float split);
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
    BoundEdge() : primNum(-1), type(EdgeType::Start) { memset(point, 0, sizeof(point)); }
    BoundEdge(float p[3], int primNum, bool starting) : primNum(primNum)
    {
        memcpy(point, p, sizeof(point));
        type = starting ? EdgeType::Start : EdgeType::End;
    }
};

void KDNode::InitLeaf(SORT_LINK **objects, BoundEdge *edges, int itemCount)
{
    this->isLeaf = true;
    for (int i = 0; i < itemCount; i++)
    {
        int         index = edges[i].primNum;
        SORT_LINK **ppLinkHead = &this->leafItems.pItemLinkHead;
        SORT_LINK **ppLinkTail = &this->leafItems.pItemLinkTail;
        LinkToLinkedListFIFO(ppLinkHead, ppLinkTail, objects[index]);
    }
}

void KDNode::InitInterior(Axis axis, float split)
{
    this->isLeaf = false;
    this->axis = axis;
    this->split = split;
}

KDNode *KDTree::BuildTreeRecursive(BoundEdge *edges, SORT_LINK **pObjectArray, int objectCount, int depth)
{
    // Initialize leaf node if termination criteria met
    if (objectCount == 0)
        return nullptr;
    if (depth == 0 || objectCount == 1)
    {
        // KDNode *pNode = KDNode::CreateLeaf(pObjectArray, edges, objectCount);
        KDNode *pNode = &m_pLeafNodes[m_allocatedLeafNodeCount];
        m_allocatedLeafNodeCount++;
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
    // KDNode *pNode = KDNode::CreateInterior((KDNode::Axis)axis, edges[mid].point[axis]);
    KDNode *pNode = &m_pInteriorNodes[m_allocatedInteriorNodeCount];
    m_allocatedInteriorNodeCount++;
    pNode->InitInterior((KDNode::Axis)axis, edges[mid].point[axis]);

    // 좌/우 서브트리 생성
    int rightObjectCount = mid / 2;
    int leftObjectCount = objectCount - rightObjectCount;
    pNode->child.left = BuildTreeRecursive(edges, pObjectArray, leftObjectCount, depth - 1);
    pNode->child.right = BuildTreeRecursive(edges + mid + 1, pObjectArray, rightObjectCount, depth - 1);

    return pNode;
}

void KDTree::InsertDynamicObjectRecursive(KDNode *pNode, const Vector3 &pos, SORT_LINK *pNew, int depth)
{
    if (depth == 0 || !pNode)
        return;
    
    if (pNode->isLeaf)
    {
        LinkToLinkedListFIFO(&pNode->leafItems.pItemLinkHead, &pNode->leafItems.pItemLinkTail, pNew);
        return;
    }

    size_t axis = (size_t)pNode->axis;

    const float *pPos = (const float *)&pos;
    if (pPos[axis] <= pNode->split)
    {
        InsertDynamicObjectRecursive(pNode->child.left, pos, pNew, depth - 1);
    }
    else
    {
        InsertDynamicObjectRecursive(pNode->child.right, pos, pNew, depth - 1);
    }
}

void KDTree::DeleteDynamicObjectRecursive(KDNode *pNode, const Vector3 &pos, SORT_LINK *pDel, int depth) 
{
    if (depth == 0 || !pNode)
        return;

    if (pNode->isLeaf)
    {
        UnLinkFromLinkedList(&pNode->leafItems.pItemLinkHead, &pNode->leafItems.pItemLinkTail, pDel);
        return;
    }

    size_t axis = (size_t)pNode->axis;

    const float *pPos = (const float *)&pos;
    if (pPos[axis] <= pNode->split)
    {
        DeleteDynamicObjectRecursive(pNode->child.left, pos, pDel, depth - 1);
    }
    else
    {
        DeleteDynamicObjectRecursive(pNode->child.right, pos, pDel, depth - 1);
    }
}

void KDTree::Cleanup()
{
    if (m_pBoundArray)
    {
        delete[] m_pBoundArray;
        m_pBoundArray = nullptr;
    }
    if (m_pLinkArray)
    {
        delete[] m_pLinkArray;
        m_pLinkArray = nullptr;
    }
    if (m_pInteriorNodes)
    {
        delete[] m_pInteriorNodes;
        m_pInteriorNodes = nullptr;
    }
    if (m_pLeafNodes)
    {
        delete[] m_pLeafNodes;
        m_pLeafNodes = nullptr;
    }
}

void KDTree::DebugPrintTreeRecursive(KDNode *pNode)
{
    if (!pNode)
        return;

    bool isLeaf = pNode->isLeaf;

    if (isLeaf)
    {
        printf("Leaf Node: ");
        SORT_LINK *pCur = pNode->leafItems.pItemLinkHead;

        int itemCount = 0;
        while (pCur)
        {
            printf("Item%d ", itemCount);
            pCur = pCur->pNext;
            itemCount++;
        }
        printf("\n");
    }
    else
    {
        printf("Interior Node[axis(%d), split(%f)]\n", (int)pNode->axis, pNode->split);
    }

    if (!pNode->isLeaf)
    {
        DebugPrintTreeRecursive(pNode->child.left);
        DebugPrintTreeRecursive(pNode->child.right);
    }
}

void KDTree::BeginCreateTree(int maxObjectCount)
{
    int maxDepth = std::round(8 + 1.3f * log2f((float)maxObjectCount));
    m_maxInteriorNodeCount = pow(2, maxDepth) - 1;
    m_maxLeafNodeCount = maxObjectCount;
    m_maxObjectCount = maxObjectCount;

    m_pBoundArray = new Bounds[maxObjectCount];
    m_pLinkArray = new SORT_LINK *[maxObjectCount];

    m_pInteriorNodes = new KDNode[m_maxInteriorNodeCount];
    m_pLeafNodes = new KDNode[m_maxLeafNodeCount];

    memset(m_pInteriorNodes, 0, sizeof(KDNode) * m_maxInteriorNodeCount);
    memset(m_pLeafNodes, 0, sizeof(KDNode) * m_maxLeafNodeCount);
}

void KDTree::InsertObject(const Bounds *pBounds, SORT_LINK *pNew)
{   
    if (m_curObjectCount > m_maxObjectCount)
    {
#ifdef _DEBUG
        __debugbreak();
#endif // _DEBUG
        return;
    }
    m_totalBounds.Expand(*pBounds);
    m_pBoundArray[m_curObjectCount] = *pBounds;
    m_pLinkArray[m_curObjectCount] = pNew;
    m_curObjectCount++;
}

void KDTree::EndCreateTree()
{
    // Bounding Edge (min/max) 초기화
    BoundEdge *pEdges = new BoundEdge[2 * m_curObjectCount];
    if (!pEdges)
    {
#ifdef _DEBUG
        __debugbreak();
#endif // _DEBUG
        return;
    }
    for (int i = 0; i < m_curObjectCount; i++)
    {
        pEdges[2 * i] = BoundEdge((float *)&m_pBoundArray[i].mins, i, true);
        pEdges[2 * i + 1] = BoundEdge((float *)&m_pBoundArray[i].maxs, i, false);
    }

    // 최대 깊이 계산
    int maxDepth = std::round(8 + 1.3f * log2f((float)m_curObjectCount));
    m_depth = maxDepth;

    // 트리 생성
    m_pRoot = BuildTreeRecursive(pEdges, m_pLinkArray, m_curObjectCount, maxDepth);

    // 리소스 해제
    if (pEdges)
    {
        delete[] pEdges;
        pEdges = nullptr;
    }
}

void KDTree::InsertDynamicObject(const Vector3 &pos, SORT_LINK *pNew) 
{
    InsertDynamicObjectRecursive(m_pRoot, pos, pNew, m_depth);
}

void KDTree::DeleteDynamicObject(const Vector3 &pos, SORT_LINK *pDel) 
{
    DeleteDynamicObjectRecursive(m_pRoot, pos, pDel, m_depth);
}

void KDTree::UpdateDynamicObject(const Vector3 &pos, SORT_LINK *pObj) 
{
    DeleteDynamicObjectRecursive(m_pRoot, pos, pObj, m_depth);
    InsertDynamicObjectRecursive(m_pRoot, pos, pObj, m_depth);
}

void KDTree::DebugPrintTree() { DebugPrintTreeRecursive(m_pRoot); }

KDTree::~KDTree() { Cleanup(); }
