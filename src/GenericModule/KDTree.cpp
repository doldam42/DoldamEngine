#include "pch.h"

#include "KDTree.h"

KDNode *KDTree::InsertNodeRecursive(KDNode *pNode, float *point, int depth, void *pItem)
{
    if (!pNode)
        return new KDNode(pItem, point);

    int axis = depth % KDNode::DIMENSION;

    if (point[axis] < pNode->point[axis])
    {
        pNode->pLeft = InsertNodeRecursive(pNode->pLeft, point, depth + 1, pItem);
    }
    else
    {
        pNode->pRight = InsertNodeRecursive(pNode->pRight, point, depth + 1, pItem);
    }

    return pNode;
}

KDNode *KDTree::FindMin(KDNode *pNode, int axis, int depth)
{
    if (!pNode)
        return nullptr;

    int currAxis = depth % KDNode::DIMENSION;

    if (currAxis == axis)
    {
        if (!pNode->pLeft)
        {
            return pNode;
        }
        return FindMin(pNode->pLeft, axis, depth + 1);
    }

    KDNode *leftMin = FindMin(pNode->pLeft, axis, depth + 1);
    KDNode *rightMin = FindMin(pNode->pRight, axis, depth + 1);

    KDNode *minNode = pNode;
    if (leftMin && leftMin->point[axis] < minNode->point[axis])
    {
        minNode = leftMin;
    }
    if (rightMin && rightMin->point[axis] < minNode->point[axis])
    {
        minNode = rightMin;
    }
    return minNode;
}

KDNode *KDTree::DeleteNodeRecursive(KDNode *pNode, float *point, int depth, void *pItem)
{
    if (!pNode)
        return nullptr;

    int axis = depth % KDNode::DIMENSION;

    // 점이 일치하는 경우
    if (memcmp(pNode->point, point, sizeof(float) * KDNode::DIMENSION) == 0)
    {
        // 오른쪽 서브트리가 있는경우
        if (pNode->pRight != nullptr)
        {
            KDNode *minNode = FindMin(pNode->pRight, axis, depth + 1);
            for (int i = 0; i < KDNode::DIMENSION; i++)
            {
                pNode->point[i] = minNode->point[i];
            }
            pNode->pRight = DeleteNodeRecursive(pNode->pRight, minNode->point, depth + 1, pItem);
        }
        // 왼쪽 서브트리가 있는경우
        else if (pNode->pLeft != nullptr)
        {
            KDNode *minNode = FindMin(pNode->pLeft, axis, depth + 1);
            for (int i = 0; i < KDNode::DIMENSION; i++)
            {
                pNode->point[i] = minNode->point[i];
            }
            pNode->pRight = DeleteNodeRecursive(pNode->pLeft, minNode->point, depth + 1, pItem);
            pNode->pLeft = nullptr;
        }
        // 리프노드인 경우
        else
        {
            delete pNode;
            return nullptr;
        }
    }
    else if (point[axis] < pNode->point[axis])
    {
        pNode->pLeft = DeleteNodeRecursive(pNode->pLeft, point, depth + 1, pItem);
    }
    else
    {
        pNode->pRight = DeleteNodeRecursive(pNode->pRight, point, depth + 1, pItem);
    }

    return pNode;
}

float KDTree::distance(const float *a, const float *b)
{
    float dist = 0.0f;
    for (UINT i = 0; i < KDNode::DIMENSION; i++)
    {
        float diff = a[i] - b[i];
        dist += diff * diff;
    }
    return sqrtf(dist);
}

KDNode *KDTree::BuildTree(float **points, int pointCount, int depth)
{
    if (pointCount == 0)
        return nullptr;

    // 현재 분할 축 계산
    int axis = depth % KDNode::DIMENSION;

    // 점들을 분할 축에 따라 정렬
    std::sort(points, points + pointCount, [axis](const float *a, const float *b) { return a[axis] < b[axis]; });

    // 중간 점 선택 (균형 이진 트리)
    size_t  mid = pointCount / 2;
    KDNode *node = new KDNode(nullptr, points[mid]);

    // 좌/우 서브트리 생성
    node->pLeft = BuildTree(points, mid, depth + 1);
    node->pRight = BuildTree(points + mid + 1, mid, depth + 1);

    return node;
}

void KDTree::NearestNeighborSearch(KDNode *pNode, const float pTarget[3], int depth, KDNode **pOutNearest,
                                   float *pOutNearestDist)
{
    if (!pNode)
        return;

    // 현재 노드의 거리 계산
    float dist = distance(pNode->point, pTarget);
    if (dist < *pOutNearestDist)
    {
        *pOutNearestDist = dist;
        *pOutNearest = pNode;
    }

    int   axis = depth % KDNode::DIMENSION;
    float diff = pTarget[axis] - pNode->point[axis];

    // 분할 하위 트리를 먼저 탐색

    KDNode *nearTree = (diff < 0) ? pNode->pLeft : pNode->pRight;
    KDNode *farTree = (diff < 0) ? pNode->pRight : pNode->pLeft;

    NearestNeighborSearch(nearTree, pTarget, depth, pOutNearest, pOutNearestDist);

    // 분할 경계를 넘어 다른 트리 탐색
    if (fabsf(diff) < *pOutNearestDist)
    {
        NearestNeighborSearch(farTree, pTarget, depth + 1, pOutNearest, pOutNearestDist);
    }
}

void KDTree::Insert(void *pItem, float *point) { InsertNodeRecursive(root, point, 0, pItem); }

void KDTree::Delete(void *pItem, float *point) { DeleteNodeRecursive(root, point, 0, pItem); }
