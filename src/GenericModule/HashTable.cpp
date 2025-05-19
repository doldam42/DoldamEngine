#include "HashTable.h"
#include "pch.h"

UINT fnv1a32(const void* pData, UINT size)
{
    constexpr UINT FNV_OFFSET_BASIS_32 = 0x811C9DC5;
    constexpr UINT FNV_PRIME_32 = 16777619;

    UINT hash = FNV_OFFSET_BASIS_32;
    const char *pEntry = (char *)pData;
    for (UINT i = 0; i < size; i++)
    {
        hash ^= (BYTE)(pEntry[i]);
        hash *= FNV_PRIME_32;
    }
    return hash;
}

UINT shift_hash(const void* pData, UINT size)
{ 
    UINT hash = 0;
    const char *pEntry = (char *)pData;
    if (size & 0x00000001)
    {
        hash += (UINT)(*(BYTE *)pEntry);
        pEntry++;
        size--;
    }
    if (!size)
        return hash;
    if (size & 0x00000002)
    {
        hash += (UINT)(*(WORD *)pEntry);
        pEntry += 2;
        size -= 2;
    }
    if (!size)
        return hash;

    size = (size >> 2);

    for (UINT i = 0; i < size; i++)
    {
        hash += *(UINT *)pEntry;
        pEntry += 4;
    }

    return hash;
}

UINT HashTable::CreateKey(const void *pData, UINT size, UINT bucketNum) const
{
    UINT keyData = fnv1a32(pData, size);
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

    UINT         index = CreateKey(pKeyData, size, m_maxBucketNum);
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
    VB_BUCKET   *pBucket = (VB_BUCKET *)pSearchHandle;
    BUCKET_DESC *pBucketDesc = pBucket->pBucketDesc;

    UnLinkFromLinkedList(&pBucketDesc->pBucketLinkHead, &pBucketDesc->pBucketLinkTail, &pBucket->sortLink);

    pBucketDesc->linkNum--;

    free(pBucket);
    m_itemNum--;
}

UINT HashTable::GetMaxBucketNum() const { return m_maxBucketNum; }

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

UINT HashTable::GetItemNum() const { return m_itemNum; }

UINT HashTable::GetItemNumInBucket(const void *pKeyData, UINT size) const
{
    UINT numItem = 0;
    if (size > m_maxKeyDataSize)
    {
        __debugbreak();
        return 0;
    }

    UINT         index = CreateKey(pKeyData, size, m_maxBucketNum);
    BUCKET_DESC *pBucketDesc = m_pBucketDescTable + index;
    SORT_LINK *pCur = pBucketDesc->pBucketLinkHead;
    while (pCur)
    {
        VB_BUCKET *pBucket = (VB_BUCKET *)pCur->pItem;
        if (memcmp(pBucket->pKeyData, pKeyData, size))
            numItem++;

        pCur = pCur->pNext;
    }

    return numItem;
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

HashTable::~HashTable() { Cleanup(); }
