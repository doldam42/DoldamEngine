#include "pch.h"

#include "D3DResourceRecycleBin.h"

D3DRESOURCE_LINK *D3DResourceRecycleBin::FindLink(UINT size) 
{ 
	UINT maxBufferCount = (UINT)_countof(m_pResourceLink); 
	D3DRESOURCE_LINK *pSelectedLink = nullptr;

	for (UINT i = 0; i < maxBufferCount; i++)
    {
        D3DRESOURCE_LINK *pLink = m_pResourceLink + i;
        if (size <= pLink->size)
        {
            pSelectedLink = pLink;
            break;
        }
    }
    return pSelectedLink;
}

UINT D3DResourceRecycleBin::UpdatePendingResource(ULONGLONG curTick)
{
    //
    // 대기 프레임 수 만큼 경과한 리소스들을 대기 링크에서 빼내서  사이즈별 할당 링크로 이동
    //
    UINT resourceCount = 0; // 대기링크에 남아있는 리소스 개수

    SORT_LINK *pCur = m_pPendingResourceLinkHead;
    SORT_LINK *pNext = nullptr;
    while (pCur)
    {
        pNext = pCur->pNext;
        resourceCount++;

        D3DRESOURCE_ALLOC_DESC *pDesc = (D3DRESOURCE_ALLOC_DESC *)pCur->pItem;

        if (pDesc->frameLifeCount <= 0)
            __debugbreak();

        int lifeCount = --pDesc->frameLifeCount;
        if (0 == lifeCount)
        {
            // 대기 링크에서 제거
            UnLinkFromLinkedList(&m_pPendingResourceLinkHead, &m_pPendingResourceLinkTail, &pDesc->Link);

            // 해당 리소스의 사이즈가 모여있는 링크에 추가
            D3DRESOURCE_LINK *pLink = FindLink((UINT)pDesc->size);
            if (!pLink)
                __debugbreak();

            pDesc->RegisteredTick = curTick; // 휴지통에 넣은 시간 기록
            pLink->resourceCount++;
            LinkToLinkedListFIFO(&pLink->pHead, &pLink->pTail, &pDesc->Link);
            resourceCount--;
        }
        pCur = pNext;
    }
    return resourceCount;
}

void D3DResourceRecycleBin::FreeAllExpiredD3DResource(ULONGLONG curTick)
{
    const ULONGLONG FREE_EXPIRED_D3DRESOURCE_TICK = 1000 * 60; // 일단 등록후 1분 경과하면 제거

    UINT maxBufferCount = (UINT)_countof(m_pResourceLink);
    for (UINT i = 0; i < maxBufferCount; i++)
    {
        D3DRESOURCE_LINK *pLink = m_pResourceLink + i;
        SORT_LINK        *pCur = pLink->pHead;
        SORT_LINK        *pNext = nullptr;
        while (pCur)
        {
            pNext = pCur->pNext;

            D3DRESOURCE_ALLOC_DESC *pDesc = (D3DRESOURCE_ALLOC_DESC *)pCur->pItem;
            if (curTick - pDesc->RegisteredTick > FREE_EXPIRED_D3DRESOURCE_TICK)
            {
                UnLinkFromLinkedList(&pLink->pHead, &pLink->pTail, &pDesc->Link);
                pDesc->pResource->Release();
                pDesc->pResource = nullptr;
#ifdef _DEBUG
                WCHAR strBuff[64] = {'\0'};
                swprintf_s(strBuff, _countof(strBuff), L"D3DResource(%u Bytes) released\n", pDesc->size);
                OutputDebugStringW(strBuff);
#endif
                delete pDesc;
                pLink->resourceCount--;
            }
            pCur = pNext;
        }
    }
}

void D3DResourceRecycleBin::Cleanup()
{
    ULONGLONG CurTick = GetTickCount64();

    while (1)
    {
        // 대기 링크에 존재하는 리소스들을 모두 사이즈별 링크로 이동
        UINT dwPendingResourceCount = UpdatePendingResource(CurTick);
        if (!dwPendingResourceCount)
            break;
    }

    // 사이즈별 링크에 들어있는 리소스들을 모두 해제
    UINT dwMaxBufferCount = (UINT)_countof(m_pResourceLink);
    for (UINT i = 0; i < dwMaxBufferCount; i++)
    {
        D3DRESOURCE_LINK *pLink = m_pResourceLink + i;
        while (pLink->pHead)
        {
            D3DRESOURCE_ALLOC_DESC *pDesc = (D3DRESOURCE_ALLOC_DESC *)pLink->pHead->pItem;
            UnLinkFromLinkedList(&pLink->pHead, &pLink->pTail, &pDesc->Link);
            pDesc->pResource->Release();
            pDesc->pResource = nullptr;
            delete pDesc;
            pLink->resourceCount--;
        }
    }
}

void D3DResourceRecycleBin::Initialize(ID3D12Device5 *pD3DDevice, D3D12_HEAP_TYPE HeapType,
                                       D3D12_RESOURCE_FLAGS ResourceFlags, D3D12_RESOURCE_STATES InitialResourceState,
                                       const WCHAR *resourceName)
{
    m_pD3DDevice = pD3DDevice;

    UINT maxBufferCount = (UINT)_countof(m_pResourceLink);
    UINT bufferSize = 65536;
    for (UINT i = 0; i < bufferSize; i++)
    {
        m_pResourceLink[i].size = bufferSize;
        bufferSize *= 2;
    }
    m_HeapType = HeapType;
    m_ResourceFlags = ResourceFlags;
    m_InitialResouceState = InitialResourceState;
    wcscpy_s(m_resourceName, resourceName);
}

ID3D12Resource* D3DResourceRecycleBin::Alloc(UINT Size)
{
    ID3D12Resource   *pResource = nullptr;
    D3DRESOURCE_LINK *pLink = FindLink(Size);
    if (!pLink)
        __debugbreak();

    if (pLink->pHead)
    {
        D3DRESOURCE_ALLOC_DESC *pDesc = (D3DRESOURCE_ALLOC_DESC *)pLink->pHead->pItem;
        UnLinkFromLinkedList(&pLink->pHead, &pLink->pTail, &pDesc->Link);
        pResource = pDesc->pResource;

        D3D12_RESOURCE_DESC desc = pResource->GetDesc();
        if (desc.Width < Size)
            __debugbreak();

        delete pDesc;
        pLink->resourceCount--;
        goto lb_return;
    }

    DASSERT(Size <= pLink->size);
    
    CD3DX12_HEAP_PROPERTIES HeapProp = CD3DX12_HEAP_PROPERTIES(m_HeapType);
    CD3DX12_RESOURCE_DESC   BufferDesc = CD3DX12_RESOURCE_DESC::Buffer(pLink->size, m_ResourceFlags);

    if (FAILED(m_pD3DDevice->CreateCommittedResource(&HeapProp, D3D12_HEAP_FLAG_NONE, &BufferDesc,
                                                     m_InitialResouceState, nullptr, IID_PPV_ARGS(&pResource))))
    {
        __debugbreak();
    }
    pResource->SetName(m_resourceName);
lb_return:
    return pResource;
}

void D3DResourceRecycleBin::Free(ID3D12Resource *pResource, int iFrameLifeCount) 
{
    D3D12_RESOURCE_DESC desc = pResource->GetDesc();

    D3DRESOURCE_ALLOC_DESC *pDesc = new D3DRESOURCE_ALLOC_DESC;
    pDesc->size = (UINT)desc.Width;
    pDesc->Link.pItem = pDesc;
    pDesc->Link.pNext = nullptr;
    pDesc->Link.pPrev = nullptr;
    pDesc->pResource = pResource;
    pDesc->frameLifeCount = iFrameLifeCount;

    LinkToLinkedListFIFO(&m_pPendingResourceLinkHead, &m_pPendingResourceLinkTail, &pDesc->Link);
}

void D3DResourceRecycleBin::Update(ULONGLONG CurTick) 
{
    // Free()로 대기 링크에 들어간 리소스들은 대기 프레임 수 만큼 경과하면 사이즈별 할당 링크로 이동
    UpdatePendingResource(CurTick);

    // 할당 링크에 존재하는 리소스들도 등록된지 일정 시간이 경과하면 제거한다.
    ULONGLONG ElapsedTick = CurTick - m_PrevFreedTick;
    if (ElapsedTick > 1000)
    {
        // 1초마다 시간이 경과된 리소스들을 실제로 release
        FreeAllExpiredD3DResource(CurTick);
        m_PrevFreedTick = CurTick;
    }
}

D3DResourceRecycleBin::~D3DResourceRecycleBin() { Cleanup(); }
