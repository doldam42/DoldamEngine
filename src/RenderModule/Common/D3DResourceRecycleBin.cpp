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
    // ��� ������ �� ��ŭ ����� ���ҽ����� ��� ��ũ���� ������  ����� �Ҵ� ��ũ�� �̵�
    //
    UINT resourceCount = 0; // ��⸵ũ�� �����ִ� ���ҽ� ����

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
            // ��� ��ũ���� ����
            UnLinkFromLinkedList(&m_pPendingResourceLinkHead, &m_pPendingResourceLinkTail, &pDesc->Link);

            // �ش� ���ҽ��� ����� ���ִ� ��ũ�� �߰�
            D3DRESOURCE_LINK *pLink = FindLink((UINT)pDesc->size);
            if (!pLink)
                __debugbreak();

            pDesc->RegisteredTick = curTick; // �����뿡 ���� �ð� ���
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
    const ULONGLONG FREE_EXPIRED_D3DRESOURCE_TICK = 1000 * 60; // �ϴ� ����� 1�� ����ϸ� ����

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
        // ��� ��ũ�� �����ϴ� ���ҽ����� ��� ����� ��ũ�� �̵�
        UINT dwPendingResourceCount = UpdatePendingResource(CurTick);
        if (!dwPendingResourceCount)
            break;
    }

    // ����� ��ũ�� ����ִ� ���ҽ����� ��� ����
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
    // Free()�� ��� ��ũ�� �� ���ҽ����� ��� ������ �� ��ŭ ����ϸ� ����� �Ҵ� ��ũ�� �̵�
    UpdatePendingResource(CurTick);

    // �Ҵ� ��ũ�� �����ϴ� ���ҽ��鵵 ��ϵ��� ���� �ð��� ����ϸ� �����Ѵ�.
    ULONGLONG ElapsedTick = CurTick - m_PrevFreedTick;
    if (ElapsedTick > 1000)
    {
        // 1�ʸ��� �ð��� ����� ���ҽ����� ������ release
        FreeAllExpiredD3DResource(CurTick);
        m_PrevFreedTick = CurTick;
    }
}

D3DResourceRecycleBin::~D3DResourceRecycleBin() { Cleanup(); }
