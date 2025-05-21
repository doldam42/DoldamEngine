#pragma once

#include <d3d12.h>

#include "../GenericModule/LinkedList.h"
#include "../GenericModule/MemoryPool.h"

class D3DResourceRecyclingBin
{
  private:
    static const size_t BASE_SIZE = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    struct D3DRESOUCRE_ALLOC_DESC
    {
        ID3D12Resource *pResource;
        SORT_LINK       Link;
        UINT            recycleCountDown;
    };
    enum ALLOC_TYPE
    {
        ALLOC_TYPE_64KB,
        ALLOC_TYPE_128KB,
        ALLOC_TYPE_256KB,
        ALLOC_TYPE_512KB,
        ALLOC_TYPE_1MB,
        ALLOC_TYPE_COUNT,
        ALLOC_TYPE_NONE
    };

    ID3D12Device *m_pDevice = nullptr;

    D3D12_HEAP_TYPE       m_resourceType;
    D3D12_HEAP_FLAGS      m_resourceFlags;
    D3D12_RESOURCE_STATES m_resourceInitialState;

    SORT_LINK *m_pD3DResourcePoolLinkHead[ALLOC_TYPE_COUNT] = {nullptr};
    SORT_LINK *m_pD3DResourcePoolLinkTail[ALLOC_TYPE_COUNT] = {nullptr};

    SORT_LINK *m_pD3DResourcePendingLinkHead = nullptr;
    SORT_LINK *m_pD3DResourcePendingLinkTail = nullptr;

    D3DRESOUCRE_ALLOC_DESC *CreateD3DResource(UINT size);

    BOOL       GetAllocTypeAndSize(UINT size, UINT *pSize, ALLOC_TYPE *pType);
    ALLOC_TYPE GetAllocTYPE(UINT size);
    UINT       GetResourceSize(UINT size);

  public:
    BOOL Initialize(ID3D12Device *pDevice, D3D12_HEAP_TYPE type, D3D12_HEAP_FLAGS flags,
                    D3D12_RESOURCE_STATES initialState);

    ID3D12Resource *Alloc(UINT size);

    void Free(ID3D12Resource *pDel, UINT recycleFrameCount);

    // 프레임 당 한번 씩 호출
    void Update();
};
