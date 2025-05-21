#include "pch.h"

#include "D3DResourceRecyclingBin.h"

D3DResourceRecyclingBin::D3DRESOUCRE_ALLOC_DESC *D3DResourceRecyclingBin::CreateD3DResource(UINT size)
{
    D3DRESOUCRE_ALLOC_DESC *pNew = new D3DRESOUCRE_ALLOC_DESC;
    ID3D12Resource         *pResource = nullptr;
    if (FAILED(m_pDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(m_resourceType), m_resourceFlags,
                                                  &CD3DX12_RESOURCE_DESC::Buffer(size), m_resourceInitialState, nullptr,
                                                  IID_PPV_ARGS(&pResource))))
    {
        __debugbreak();
    }

    pNew->Link.pItem = pNew;
    pNew->Link.pNext = nullptr;
    pNew->Link.pPrev = nullptr;
    pNew->pResource = pResource;

    return pNew;
}

BOOL D3DResourceRecyclingBin::GetAllocTypeAndSize(UINT size, UINT *pSize, ALLOC_TYPE *pType)
{
    
    if (size <= BASE_SIZE)
    {
        *pType = ALLOC_TYPE_64KB;
        *pSize = BASE_SIZE;
        return TRUE;
    }
    if (size <= BASE_SIZE * 2)
    {
        *pType = ALLOC_TYPE_128KB;
        *pSize = BASE_SIZE * 2;
        return TRUE;
    }
    if (size <= BASE_SIZE * 4)
    {
        *pType = ALLOC_TYPE_256KB;
        *pSize = BASE_SIZE * 4;
        return TRUE;
    }
    if (size <= BASE_SIZE * 8)
    {
        *pType = ALLOC_TYPE_512KB;
        *pSize = BASE_SIZE * 8;
        return TRUE;
    }
    if (size <= BASE_SIZE * 16)
    {
        *pType = ALLOC_TYPE_1MB;
        *pSize = BASE_SIZE * 16;
        return TRUE;
    }

    DASSERT(false);
    return FALSE;
}

D3DResourceRecyclingBin::ALLOC_TYPE D3DResourceRecyclingBin::GetAllocTYPE(UINT size)
{
    if (size <= BASE_SIZE)
    {
        return ALLOC_TYPE_64KB;
    }
    if (size <= BASE_SIZE * 2)
    {
        return ALLOC_TYPE_128KB;
    }
    if (size <= BASE_SIZE * 4)
    {
        return ALLOC_TYPE_256KB;
    }
    if (size <= BASE_SIZE * 8)
    {
        return ALLOC_TYPE_512KB;
    }
    if (size <= BASE_SIZE * 16)
    {
        return ALLOC_TYPE_1MB;
    }
    DASSERT(false);
    return ALLOC_TYPE_NONE;
}

UINT D3DResourceRecyclingBin::GetResourceSize(UINT size)
{
    if (size <= BASE_SIZE)
    {
        return BASE_SIZE;
    }
    if (size <= BASE_SIZE * 2)
    {
        return BASE_SIZE * 2;
    }
    if (size <= BASE_SIZE * 4)
    {
        return BASE_SIZE * 4;
    }
    if (size <= BASE_SIZE * 8)
    {
        return BASE_SIZE * 8;
    }
    if (size <= BASE_SIZE * 16)
    {
        return BASE_SIZE * 16;
    }
    
    DASSERT(false);
    return 0;
}

BOOL D3DResourceRecyclingBin::Initialize(ID3D12Device *pDevice, D3D12_HEAP_TYPE type, D3D12_HEAP_FLAGS flags,
                                           D3D12_RESOURCE_STATES initialState)
{
    m_pDevice = pDevice;
    m_resourceType = type;
    m_resourceFlags = flags;
    m_resourceInitialState = initialState;

    return TRUE;
}

ID3D12Resource *D3DResourceRecyclingBin::Alloc(UINT size)
{
    UINT resourceSIze;
    ALLOC_TYPE type;
    GetAllocTypeAndSize(size, &resourceSIze, &type);

    ID3D12Resource *pResult = nullptr;
    if (!m_pD3DResourcePoolLinkHead[type])
    {
        if (FAILED(m_pDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(m_resourceType), m_resourceFlags,
                                                      &CD3DX12_RESOURCE_DESC::Buffer(resourceSIze),
                                                      m_resourceInitialState, nullptr, IID_PPV_ARGS(&pResult))))
        {
            __debugbreak();
        }
        return pResult;
    }

    D3DRESOUCRE_ALLOC_DESC *pDel = (D3DRESOUCRE_ALLOC_DESC *)m_pD3DResourcePoolLinkHead[type]->pItem;

    if (pDel)
    {
        UnLinkFromLinkedList(&m_pD3DResourcePoolLinkHead[type], &m_pD3DResourcePoolLinkTail[type], &pDel->Link);
        pResult = pDel->pResource;
        delete pDel;
        pDel = nullptr;
    }
    
    return pResult;
}

void D3DResourceRecyclingBin::Free(ID3D12Resource *pDel, UINT recycleFrameCount) 
{
    DASSERT(pDel);

    D3D12_RESOURCE_DESC desc = pDel->GetDesc();
    ALLOC_TYPE          type = GetAllocTYPE(desc.Width);

    D3DRESOUCRE_ALLOC_DESC *pItem = new D3DRESOUCRE_ALLOC_DESC;
    pItem->Link.pItem = pItem;
    pItem->Link.pNext = nullptr;
    pItem->Link.pPrev = nullptr;
    pItem->pResource = pDel;
    pItem->recycleCountDown = recycleFrameCount;

    LinkToLinkedList(&m_pD3DResourcePendingLinkHead, &m_pD3DResourcePendingLinkTail, &pItem->Link);
}

void D3DResourceRecyclingBin::Update() 
{ 
    SORT_LINK *pCur = m_pD3DResourcePendingLinkHead;

    while (pCur)
    {
        D3DRESOUCRE_ALLOC_DESC *pItem = (D3DRESOUCRE_ALLOC_DESC *)pCur->pItem;
        pCur = pCur->pNext;

        pItem->recycleCountDown--;
        if (pItem->recycleCountDown == 0)
        {
            D3D12_RESOURCE_DESC desc = pItem->pResource->GetDesc();
            ALLOC_TYPE          type = GetAllocTYPE(desc.Width);
            UnLinkFromLinkedList(&m_pD3DResourcePendingLinkHead, &m_pD3DResourcePendingLinkTail, &pItem->Link);
            LinkToLinkedList(&m_pD3DResourcePoolLinkHead[type], &m_pD3DResourcePoolLinkTail[type], &pItem->Link);
        }
    }
}
