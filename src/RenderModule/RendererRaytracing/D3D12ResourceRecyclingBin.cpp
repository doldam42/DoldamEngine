#include "D3D12ResourceRecyclingBin.h"
#include "pch.h"

D3D12ResourceRecyclingBin::D3DRESOUCRE_ALLOC_DESC *D3D12ResourceRecyclingBin::CreateD3DResource(UINT size)
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

BOOL D3D12ResourceRecyclingBin::GetAllocTypeAndSize(UINT size, UINT *pSize, ALLOC_TYPE *pType)
{
    if (size <= 1024 * 4)
    {
        *pType = ALLOC_TYPE_4KB;
        *pSize = 1024 * 4;
        return TRUE;
    }
    if (size <= 1024 * 16)
    {
        *pType = ALLOC_TYPE_16KB;
        *pSize = 1024 * 4;
        return TRUE;
    }
    if (size <= 1024 * 64)
    {
        *pType = ALLOC_TYPE_64KB;
        *pSize = 1024 * 4;
        return TRUE;
    }
    if (size <= 1024 * 256)
    {
        *pType = ALLOC_TYPE_256KB;
        *pSize = 1024 * 4;
        return TRUE;
    }
    if (size <= 1024 * 1024)
    {
        *pType = ALLOC_TYPE_1MB;
        *pSize = 1024 * 4;
        return TRUE;
    }

    DASSERT(false);
    return FALSE;
}

D3D12ResourceRecyclingBin::ALLOC_TYPE D3D12ResourceRecyclingBin::GetAllocTYPE(UINT size)
{
    if (size <= 1024 * 4)
    {
        return ALLOC_TYPE_4KB;
    }
    if (size <= 1024 * 16)
    {
        return ALLOC_TYPE_16KB;
    }
    if (size <= 1024 * 64)
    {
        return ALLOC_TYPE_64KB;
    }
    if (size <= 1024 * 256)
    {
        return ALLOC_TYPE_256KB;
    }
    if (size <= 1024 * 1024)
    {
        return ALLOC_TYPE_1MB;
    }
    DASSERT(false);
    return ALLOC_TYPE_NONE;
}

UINT D3D12ResourceRecyclingBin::GetResourceSize(UINT size)
{
    if (size <= 1024 * 4)
    {
        return 1024 * 4;
    }
    if (size <= 1024 * 16)
    {
        return 1024 * 16;
    }
    if (size <= 1024 * 64)
    {
        return 1024 * 64;
    }
    if (size <= 1024 * 256)
    {
        return 1024 * 256;
    }
    if (size <= 1024 * 1024)
    {
        return 1024 * 1024;
    }
    DASSERT(false);
    return 0;
}

BOOL D3D12ResourceRecyclingBin::Initialize(ID3D12Device *pDevice, D3D12_HEAP_TYPE type, D3D12_HEAP_FLAGS flags,
                                           D3D12_RESOURCE_STATES initialState)
{
    m_pDevice = pDevice;
    m_resourceType = type;
    m_resourceFlags = flags;
    m_resourceInitialState = initialState;
}

ID3D12Resource *D3D12ResourceRecyclingBin::Alloc(UINT size)
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

    UnLinkFromLinkedList(&m_pD3DResourcePoolLinkHead[type], &m_pD3DResourcePoolLinkTail[type],
                         m_pD3DResourcePoolLinkHead[type]);

    if (pDel)
    {
        pResult = pDel->pResource;
        delete pDel;
        pDel = nullptr;
    }
    
    return pResult;
}

void D3D12ResourceRecyclingBin::Free(ID3D12Resource *pDel) 
{
    if (pDel)
    {
        __debugbreak();
        return;
    }

    D3D12_RESOURCE_DESC desc = pDel->GetDesc();
    ALLOC_TYPE          type = GetAllocTYPE(desc.Width);
    
    D3DRESOUCRE_ALLOC_DESC *pItem = new D3DRESOUCRE_ALLOC_DESC;
    pItem->Link.pItem = pItem;
    pItem->Link.pNext = nullptr;
    pItem->Link.pPrev = nullptr;
    pItem->pResource = pDel;

    LinkToLinkedList(&m_pD3DResourcePoolLinkHead[type], &m_pD3DResourcePoolLinkTail[type], &pItem->Link);
}
