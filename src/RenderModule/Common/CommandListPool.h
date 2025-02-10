#pragma once

struct COMMAND_LIST
{
    ID3D12CommandAllocator    *pDirectCommandAllocator;
    ID3D12GraphicsCommandList4 *pDirectCommandList;
    SORT_LINK                  link;
    BOOL                       isClosed;
};

class CommandListPool
{
    ID3D12Device5          *m_pD3DDevice = nullptr;
    D3D12_COMMAND_LIST_TYPE m_commandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;
    UINT                    m_allocatedCmdNum = 0;
    UINT                    m_availableCmdNum = 0;
    UINT                    m_totalCmdNum = 0;
    UINT                    m_maxCmdListNum = 0;
    COMMAND_LIST           *m_pCurCmdList = nullptr;
    SORT_LINK              *m_pAllocatedCmdLinkHead = nullptr;
    SORT_LINK              *m_pAllocatedCmdLinkTail = nullptr;
    SORT_LINK              *m_pAvailableCmdLinkHead = nullptr;
    SORT_LINK              *m_pAvailableCmdLinkTail = nullptr;

    BOOL          AddCmdList();
    COMMAND_LIST *AllocCmdList();
    void          Cleanup();

  public:
    BOOL                       Initialize(ID3D12Device5 *pDevice, D3D12_COMMAND_LIST_TYPE type, UINT maxCommandListNum);
    ID3D12GraphicsCommandList4 *GetCurrentCommandList();
    void                       Close();
    void                       CloseAndExecute(ID3D12CommandQueue *pCommandQueue);
    void                       Reset();

    UINT GetTotalCmdListNum() const
    {
        return m_totalCmdNum;
    }
    UINT GetAllocatedCmdListNum() const
    {
        return m_allocatedCmdNum;
    }
    UINT GetAvailableCmdListNum() const
    {
        return m_availableCmdNum;
    }
    ID3D12Device *INL_GetD3DDevice()
    {
        return m_pD3DDevice;
    }

    CommandListPool() = default;
    ~CommandListPool();
};