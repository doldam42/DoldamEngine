#pragma once
class MemoryPool
{
    uint8_t *m_pBaseAddress = nullptr;
    UINT    *m_pIndexTable = nullptr;

    UINT m_maxItemNum = 0;
    UINT m_allocatedItemCount = 0;
    UINT m_sizeInBytes = 0;
    BOOL m_hasBaseMemory = TRUE;

    void Cleanup();

  public:
    void Initialize(UINT sizeInBytes, UINT maxItemNum);
    void Initialize(void *pBaseAddr, UINT sizeInBytes, UINT maxItemNum);

    BOOL Has(void *pInAddr);

    void *Alloc();
    void  Dealloc(void *pInAddr);

    UINT  GetIndexOf(void *pInAddr);
    void *GetAddressOf(UINT index);

    void *Data() { return m_pBaseAddress; };

    MemoryPool() = default;
    ~MemoryPool();
};
