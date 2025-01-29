#include "pch.h"

#include "CommandListPool.h"

BOOL CommandListPool::AddCmdList()
{
    bool result = FALSE;
    COMMAND_LIST              *pCmdList = nullptr;
    ID3D12CommandAllocator    *pCommandAllocator = nullptr;
    ID3D12GraphicsCommandList4 *pCommandList = nullptr;

    if (m_totalCmdNum >= m_maxCmdListNum)
    {
        __debugbreak();
        goto lb_return;
    }

    if (FAILED(m_pD3DDevice->CreateCommandAllocator(m_commandListType, IID_PPV_ARGS(&pCommandAllocator))))
    {
        __debugbreak();
        goto lb_return;
    }

    // Create the command list.
    if (FAILED(m_pD3DDevice->CreateCommandList(0, m_commandListType, pCommandAllocator, nullptr,
                                               IID_PPV_ARGS(&pCommandList))))
    {
        __debugbreak();
        goto lb_return;
    }

    // Command lists are created in the recording state, but there is nothing
    // to record yet. The main loop expects it to be closed, so close it now.
    pCmdList = new COMMAND_LIST;
    memset(pCmdList, 0, sizeof(COMMAND_LIST));
    pCmdList->pDirectCommandAllocator = pCommandAllocator;
    pCmdList->pDirectCommandList = pCommandList;
    pCmdList->link.pItem = pCmdList;
    m_totalCmdNum++;

    LinkToLinkedList(&m_pAvailableCmdLinkHead, &m_pAvailableCmdLinkTail, &pCmdList->link);
    result = TRUE;
lb_return:
    return result;
}

COMMAND_LIST *CommandListPool::AllocCmdList()
{
    COMMAND_LIST *pCmdList = nullptr;

    if (!m_pAvailableCmdLinkHead)
    {
        if (!AddCmdList())
        {
            goto lb_return;
        }
    }

    pCmdList = (COMMAND_LIST *)m_pAvailableCmdLinkHead->pItem;

    UnLinkFromLinkedList(&m_pAvailableCmdLinkHead, &m_pAvailableCmdLinkTail, &pCmdList->link);
    m_availableCmdNum--;

    LinkToLinkedList(&m_pAllocatedCmdLinkHead, &m_pAllocatedCmdLinkTail, &pCmdList->link);
    m_allocatedCmdNum++;

lb_return:
    return pCmdList;
}

void CommandListPool::Cleanup()
{
    Reset();

    while (m_pAvailableCmdLinkHead)
    {
        COMMAND_LIST *pCmdList = (COMMAND_LIST *)m_pAvailableCmdLinkHead->pItem;
        pCmdList->pDirectCommandList->Release();
        pCmdList->pDirectCommandList = nullptr;

        pCmdList->pDirectCommandAllocator->Release();
        pCmdList->pDirectCommandAllocator = nullptr;
        m_totalCmdNum--;

        UnLinkFromLinkedList(&m_pAvailableCmdLinkHead, &m_pAvailableCmdLinkTail, &pCmdList->link);
        m_availableCmdNum--;

        delete pCmdList;
    }
}

BOOL CommandListPool::Initialize(ID3D12Device5 *pDevice, D3D12_COMMAND_LIST_TYPE type, UINT maxCommandListNum)
{
    if (maxCommandListNum < 2)
        __debugbreak();

    m_pD3DDevice = pDevice;
    m_commandListType = type;
    m_maxCmdListNum = maxCommandListNum;

    return TRUE;
}

ID3D12GraphicsCommandList4 *CommandListPool::GetCurrentCommandList()
{
    if (!m_pCurCmdList)
    {
        m_pCurCmdList = AllocCmdList();
        if (!m_pCurCmdList)
        {
            __debugbreak();
        }
    }
    return m_pCurCmdList->pDirectCommandList;
}

void CommandListPool::Close()
{
    if (!m_pCurCmdList)
        __debugbreak();
    if (m_pCurCmdList->isClosed)
        __debugbreak();
    if (FAILED(m_pCurCmdList->pDirectCommandList->Close()))
        __debugbreak();

    m_pCurCmdList->isClosed = TRUE;
    m_pCurCmdList = nullptr;
}

void CommandListPool::CloseAndExecute(ID3D12CommandQueue *pCommandQueue)
{
    if (!m_pCurCmdList)
        __debugbreak();

    if (m_pCurCmdList->isClosed)
        __debugbreak();

    if (FAILED(m_pCurCmdList->pDirectCommandList->Close()))
        __debugbreak();

    m_pCurCmdList->isClosed = TRUE;

    pCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList **)&m_pCurCmdList->pDirectCommandList);
    m_pCurCmdList = nullptr;
}

void CommandListPool::Reset()
{
    while (m_pAllocatedCmdLinkHead)
    {
        COMMAND_LIST *pCmdList = (COMMAND_LIST *)m_pAllocatedCmdLinkHead->pItem;

        if (FAILED(pCmdList->pDirectCommandAllocator->Reset()))
            __debugbreak();

        if (FAILED(pCmdList->pDirectCommandList->Reset(pCmdList->pDirectCommandAllocator, nullptr)))
            __debugbreak();

        pCmdList->isClosed = FALSE;

        UnLinkFromLinkedList(&m_pAllocatedCmdLinkHead, &m_pAllocatedCmdLinkTail, &pCmdList->link);
        m_allocatedCmdNum--;

        LinkToLinkedListFIFO(&m_pAvailableCmdLinkHead, &m_pAvailableCmdLinkTail, &pCmdList->link);
        m_availableCmdNum++;
    }
}

CommandListPool::~CommandListPool()
{
    Cleanup();
}
