#pragma once

#include <d3d12.h>

#include "../GenericModule/LinkedList.h"
#include "../GenericModule/MemoryPool.h"

class D3D12ResourceRecyclingBin
{ 
  private:
    struct D3DRESOUCRE_ALLOC_DESC
    {
        ID3D12Resource *pResource;
        SORT_LINK       Link;     
    };
    enum ALLOC_TYPE
    {
        ALLOC_TYPE_4KB = 0,
        ALLOC_TYPE_16KB,
        ALLOC_TYPE_64KB,
        ALLOC_TYPE_256KB,
        ALLOC_TYPE_1MB,
        ALLOC_TYPE_COUNT,
        ALLOC_TYPE_NONE
    };

    ID3D12Device *m_pDevice = nullptr;

    D3D12_HEAP_TYPE m_resourceType;
    D3D12_HEAP_FLAGS m_resourceFlags;
    D3D12_RESOURCE_STATES m_resourceInitialState;

    SORT_LINK *m_pD3DResourcePoolLinkHead[ALLOC_TYPE_COUNT];
    SORT_LINK *m_pD3DResourcePoolLinkTail[ALLOC_TYPE_COUNT];
    
    D3DRESOUCRE_ALLOC_DESC *CreateD3DResource(UINT size);

    BOOL       GetAllocTypeAndSize(UINT size, UINT *pSize, ALLOC_TYPE *pType);
    ALLOC_TYPE GetAllocTYPE(UINT size);
    UINT       GetResourceSize(UINT size);

  public:
    BOOL Initialize(ID3D12Device* pDevice, D3D12_HEAP_TYPE type, D3D12_HEAP_FLAGS flags, D3D12_RESOURCE_STATES initialState);

    ID3D12Resource *Alloc(UINT size);
    void Free(ID3D12Resource* pDel);
};
