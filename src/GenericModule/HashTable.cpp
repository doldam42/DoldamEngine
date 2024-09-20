#include "pch.h"
#include "HashTable.h"

UINT HashTable::CreateKey(const void *pData, UINT size, UINT bucketNum)
{
    UINT keyData = 0;

    const char *pEntry = (char *)pData;
    if (size & 0x00000001)
    {
        keyData += (UINT)(*(BYTE *)pEntry);
        pEntry++;
        size--;
    }
    if (!size)
        goto lb_exit;
    if (size & 0x00000002)
    {
        keyData += (UINT)(*(WORD *)pEntry);
        pEntry += 2;
        size -= 2;
    }
    if (!size)
        goto lb_exit;

    size = (size >> 2);

    for (UINT i = 0; i < size; i++)
    {
        keyData += *(UINT *)pEntry;
        pEntry += 4;
    }

lb_exit:
    UINT index = keyData % bucketNum;

    return index;
}

BOOL HashTable::Initialize(UINT maxBucketNum, UINT maxKeySize, UINT maxItemNum)
{
    m_maxKeyDataSize = maxKeySize;
    m_maxBucketNum = maxBucketNum;

    m_pBucketDescTable = new BUCKET_DESC[maxBucketNum];
    memset(m_pBucketDescTable, 0, sizeof(BUCKET_DESC) * maxBucketNum);

    return TRUE;
}

UINT HashTable::Select(void **ppOutItemList, UINT maxItemNum, const void *pKeyData, UINT size)
{
    UINT         selectedItemNum = 0;
    UINT         index = CreateKey(pKeyData, size, m_maxBucketNum);
    BUCKET_DESC *pBucketDesc = m_pBucketDescTable + index;

    SORT_LINK *pCur = pBucketDesc->pBucketLinkHead;

    VB_BUCKET *pBucket;

    while (pCur)
    {
        if (!maxItemNum)
            goto lb_return;

        pBucket = (VB_BUCKET *)pCur->pItem;

        if (pBucket->size != size)
            goto lb_next;

        if (memcmp(pBucket->pKeyData, pKeyData, size))
            goto lb_next;

        maxItemNum--;

        ppOutItemList[selectedItemNum] = (void *)pBucket->pItem;
        selectedItemNum++;

    lb_next:
        pCur = pCur->pNext;
    }

lb_return:
    return selectedItemNum;
}

void *HashTable::Insert(const void *pItem, const void *pKeyData, UINT size)
{
    void *pSearchHandle = nullptr;

    if (size > m_maxKeyDataSize)
    {
        __debugbreak();
        goto lb_return;
    }

    UINT       bucketMemSize = (UINT)(sizeof(VB_BUCKET)) + m_maxKeyDataSize;
    VB_BUCKET *pBucket = (VB_BUCKET *)malloc(bucketMemSize);

    UINT index = CreateKey(pKeyData, size, m_maxBucketNum);
    BUCKET_DESC *pBucketDesc = m_pBucketDescTable + index;

    pBucket->pItem = pItem;
    pBucket->size = size;
    pBucket->pBucketDesc = pBucketDesc;
    pBucket->sortLink.pPrev = nullptr;
    pBucket->sortLink.pNext = nullptr;
    pBucket->sortLink.pItem = pBucket;
    pBucketDesc->linkNum++;

    memcpy(pBucket->pKeyData, pKeyData, size);

    LinkToLinkedListFIFO(&pBucketDesc->pBucketLinkHead, &pBucketDesc->pBucketLinkTail, &pBucket->sortLink);

    m_itemNum++;
    pSearchHandle = pBucket;

lb_return:
    return pSearchHandle;
}

void HashTable::Delete(const void *pSearchHandle)
{
    VB_BUCKET *pBucket = (VB_BUCKET *)pSearchHandle;
    BUCKET_DESC *pBucketDesc = pBucket->pBucketDesc;

    UnLinkFromLinkedList(&pBucketDesc->pBucketLinkHead, &pBucketDesc->pBucketLinkTail, &pBucket->sortLink);

    pBucketDesc->linkNum--;

    free(pBucket);
    m_itemNum--;
}

UINT HashTable::GetMaxBucketNum() const
{
    return m_maxBucketNum;
}

void HashTable::DeleteAll()
{
    VB_BUCKET *pBucket;

    for (UINT i = 0; i < m_maxBucketNum; i++)
    {
        while (m_pBucketDescTable[i].pBucketLinkHead)
        {
            pBucket = (VB_BUCKET *)m_pBucketDescTable[i].pBucketLinkHead->pItem;
            Delete(pBucket);
        }
    }
}

UINT HashTable::GetAllItems(void **ppOutItemList, UINT maxItemNum, BOOL *pbOutInsufficient) const
{
    VB_BUCKET *pBucket;
    SORT_LINK *pCur;

    *pbOutInsufficient = FALSE;
    UINT itemNum = 0;

    for (UINT i = 0; i < m_maxBucketNum; i++)
    {
        pCur = m_pBucketDescTable[i].pBucketLinkHead;
        while (pCur)
        {
            pBucket = (VB_BUCKET *)pCur->pItem;

            if (itemNum >= maxItemNum)
            {
                *pbOutInsufficient = TRUE;
                goto lb_return;
            }

            ppOutItemList[itemNum] = (void *)pBucket->pItem;
            itemNum++;

            pCur = pCur->pNext;
        }
    }

lb_return:
    return itemNum;
}

UINT HashTable::GetKeyPtrAndSize(void **ppOutKeyPtr, const void *pSearchHandle) const
{
    *ppOutKeyPtr = ((VB_BUCKET *)pSearchHandle)->pKeyData;
    UINT size = ((VB_BUCKET *)pSearchHandle)->size;

    return size;
}

UINT HashTable::GetItemNum() const
{
    return m_itemNum;
}

void HashTable::ResourceCheck() const
{
    if (m_itemNum)
        __debugbreak();
}

void HashTable::Cleanup()
{
    ResourceCheck();

    DeleteAll();
    if (m_pBucketDescTable)
    {
        delete[] m_pBucketDescTable;
        m_pBucketDescTable = nullptr;
    }
}

HashTable::~HashTable()
{
    Cleanup();
}
