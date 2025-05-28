#pragma once

struct D3DRESOURCE_LINK
{
    SORT_LINK *pHead;
    SORT_LINK *pTail;
    UINT       size;
    UINT       resourceCount;
};

struct D3DRESOURCE_ALLOC_DESC
{
    ID3D12Resource *pResource;
    ULONGLONG       RegisteredTick;
    SORT_LINK       Link;
    UINT            size;
    int             frameLifeCount;
};

class D3DResourceRecycleBin
{
    ID3D12Device5   *m_pD3DDevice = nullptr;
    D3DRESOURCE_LINK m_pResourceLink[16] = {};

    SORT_LINK *m_pPendingResourceLinkHead = nullptr;
    SORT_LINK *m_pPendingResourceLinkTail = nullptr;

    D3D12_HEAP_TYPE       m_HeapType = D3D12_HEAP_TYPE_UPLOAD;
    D3D12_RESOURCE_FLAGS  m_ResourceFlags = D3D12_RESOURCE_FLAG_NONE;
    D3D12_RESOURCE_STATES m_InitialResouceState = D3D12_RESOURCE_STATE_GENERIC_READ;

    ULONGLONG m_PrevFreedTick = 0;
    WCHAR     m_resourceName[MAX_NAME] = {L'\0'};

    D3DRESOURCE_LINK *FindLink(UINT size);

    UINT UpdatePendingResource(ULONGLONG curTick);
    void FreeAllExpiredD3DResource(ULONGLONG curTick);
    void Cleanup();

  public:
    void            Initialize(ID3D12Device5 *pD3DDevice, D3D12_HEAP_TYPE HeapType, D3D12_RESOURCE_FLAGS ResourceFlags,
                               D3D12_RESOURCE_STATES InitialResourceState, const WCHAR *resourceName);
    ID3D12Resource *Alloc(UINT Size);
    void            Free(ID3D12Resource *pResource, int iFrameLifeCount);
    void            Update(ULONGLONG CurTick);

    D3DResourceRecycleBin() = default;
    ~D3DResourceRecycleBin();
};