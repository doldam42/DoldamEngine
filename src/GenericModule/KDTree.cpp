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

    // ���� ��ġ�ϴ� ���
    if (memcmp(pNode->point, point, sizeof(float) * KDNode::DIMENSION) == 0)
    {
        // ������ ����Ʈ���� �ִ°��
        if (pNode->pRight != nullptr)
        {
            KDNode *minNode = FindMin(pNode->pRight, axis, depth + 1);
            for (int i = 0; i < KDNode::DIMENSION; i++)
            {
                pNode->point[i] = minNode->point[i];
            }
            pNode->pRight = DeleteNodeRecursive(pNode->pRight, minNode->point, depth + 1, pItem);
        }
        // ���� ����Ʈ���� �ִ°��
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
        // ��������� ���
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

    // ���� ���� �� ���
    int axis = depth % KDNode::DIMENSION;

    // ������ ���� �࿡ ���� ����
    std::sort(points, points + pointCount, [axis](const float *a, const float *b) { return a[axis] < b[axis]; });

    // �߰� �� ���� (���� ���� Ʈ��)
    size_t  mid = pointCount / 2;
    KDNode *node = new KDNode(nullptr, points[mid]);

    // ��/�� ����Ʈ�� ����
    node->pLeft = BuildTree(points, mid, depth + 1);
    node->pRight = BuildTree(points + mid + 1, mid, depth + 1);

    return node;
}

void KDTree::NearestNeighborSearch(KDNode *pNode, const float pTarget[3], int depth, KDNode **pOutNearest,
                                   float *pOutNearestDist)
{
    if (!pNode)
        return;

    // ���� ����� �Ÿ� ���
    float dist = distance(pNode->point, pTarget);
    if (dist < *pOutNearestDist)
    {
        *pOutNearestDist = dist;
        *pOutNearest = pNode;
    }

    int   axis = depth % KDNode::DIMENSION;
    float diff = pTarget[axis] - pNode->point[axis];

    // ���� ���� Ʈ���� ���� Ž��

    KDNode *nearTree = (diff < 0) ? pNode->pLeft : pNode->pRight;
    KDNode *farTree = (diff < 0) ? pNode->pRight : pNode->pLeft;

    NearestNeighborSearch(nearTree, pTarget, depth, pOutNearest, pOutNearestDist);

    // ���� ��踦 �Ѿ� �ٸ� Ʈ�� Ž��
    if (fabsf(diff) < *pOutNearestDist)
    {
        NearestNeighborSearch(farTree, pTarget, depth + 1, pOutNearest, pOutNearestDist);
    }
}

void KDTree::Insert(void *pItem, float *point) { InsertNodeRecursive(root, point, 0, pItem); }

void KDTree::Delete(void *pItem, float *point) { DeleteNodeRecursive(root, point, 0, pItem); }
