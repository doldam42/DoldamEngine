#pragma once

struct BUCKET_DESC
{
    SORT_LINK *pBucketLinkHead;
    SORT_LINK *pBucketLinkTail;
    UINT       linkNum;
};

struct VB_BUCKET
{
    const void  *pItem;
    BUCKET_DESC *pBucketDesc;
    SORT_LINK    sortLink;
    UINT         size;
    char         pKeyData[];
};

class HashTable
{
    BUCKET_DESC *m_pBucketDescTable = nullptr;
    UINT         m_maxBucketNum = 0;
    UINT         m_maxKeyDataSize = 0;
    UINT         m_itemNum = 0;

    UINT CreateKey(const void *pData, UINT size, UINT bucketNum) const;

  public:
    BOOL  Initialize(UINT maxBucketNum, UINT maxKeySize, UINT maxItemNum);
    UINT  Select(void **ppOutItemList, UINT maxItemNum, const void *pKeyData, UINT size);
    void *Insert(const void *pItem, const void *pKeyData, UINT size);
    void  Delete(const void *pSearchHandle);
    UINT  GetMaxBucketNum() const;
    void  DeleteAll();
    UINT  GetAllItems(void **ppOutItemList, UINT maxItemNum, BOOL *pbOutInsufficient) const;
    UINT  GetKeyPtrAndSize(void **ppOutKeyPtr, const void *pSearchHandle) const;
    UINT  GetItemNum() const;
    UINT  GetItemNumInBucket(const void *pKeyData, UINT size) const;
    void  ResourceCheck() const;

    void Cleanup();

    HashTable() = default;
    ~HashTable();
};
