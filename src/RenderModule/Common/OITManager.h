#pragma once

#include "DescriptorAllocator.h"
//
//struct FragmentListNode
//{
//    UINT  next;
//    float depth;
//    UINT  color;
//};
//
enum OIT_DESCRIPTOR_INDEX
{
    OIT_DESCRIPTOR_INDEX_FIRST_NODE_ADDRESS_UAV = 0,
    OIT_DESCRIPTOR_INDEX_FRAGMENT_LIST_NODE_UAV,
    OIT_DESCRIPTOR_INDEX_FIRST_NODE_ADDRESS_SRV,
    OIT_DESCRIPTOR_INDEX_FRAGMENT_LIST_NODE_SRV,
    OIT_DESCRIPTOR_COUNT
};

class D3D12Renderer;
class OITManager
{
    static const UINT MAX_PENDING_COUNT = 2;

    D3D12Renderer *m_pRenderer = nullptr;
    ID3D12Device5 *m_pD3DDevice = nullptr;

    ID3D12Resource *m_pUAVCounterClearResource = nullptr;
    ID3D12Resource *m_pReadbackBuffers[MAX_PENDING_COUNT] = {nullptr};

    ID3D12Resource *m_pFragmentListFirstNodeAddress = nullptr;
    ID3D12Resource *m_pFragmentList = nullptr;

    DESCRIPTOR_HANDLE m_descriptorTable = {};

    UINT m_maxFragmentListNodeCount = 0;
    UINT m_allocatedNodeCount = 0;
    UINT m_curContextIndex = 0;
    UINT m_srvDescriptorSize = 0;

    void CreatDescriptorTables();
    void CreateUAVCounterClearResource();

    void CreateBuffers(UINT width, UINT height);
    void CleanupBuffers();

    void CopyUAVCounterForRead(ID3D12GraphicsCommandList *pCommandList);

    void Cleanup();

  public:
    BOOL Initialize(D3D12Renderer *pRenderer, UINT maxFragmentListNodeCount, UINT width, UINT height);

    void OnUpdateWindowSize(UINT width, UINT height);

    void SetRootDescriptorTables(UINT ThreadIndex, ID3D12GraphicsCommandList *pCommandList);

    void BeginRender(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList);
    void ResolveOIT(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList, D3D12_VIEWPORT *pViewport,
                    D3D12_RECT *pScissorRect, D3D12_CPU_DESCRIPTOR_HANDLE renderTarget);
    void EndRender();
};