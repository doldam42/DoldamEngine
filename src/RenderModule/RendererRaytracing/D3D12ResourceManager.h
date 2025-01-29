#pragma once

#include "RendererTypedef.h"
enum TEXTURE_FILE_FORMAT
{
    TEXTURE_FILE_FORMAT_M
};

class DescriptorAllocator;
class D3D12ResourceManager
{
    static const UINT DESCRIPTOR_COUNT_PER_RTV = 32;
    static const UINT DESCRIPTOR_COUNT_PER_DSV = 32;
    static const UINT DESCRIPTOR_POOL_SIZE = 6;
    static const UINT RTV_DESCRIPTOR_POOL_SIZE = 4;

    ID3D12Device              *m_pD3DDevice = nullptr;
    ID3D12CommandQueue        *m_pCommandQueue = nullptr;
    ID3D12CommandAllocator    *m_pCommandAllocator = nullptr;
    ID3D12GraphicsCommandList *m_pCommandList = nullptr;

    HANDLE       m_hFenceEvent = nullptr;
    ID3D12Fence *m_pFence = nullptr;
    UINT64       m_ui64FenceValue = 0;

    DescriptorAllocator *m_pDescriptorAllocators[DESCRIPTOR_POOL_SIZE] = {nullptr};
    DescriptorAllocator *m_pRTVDescriptorAllocators[RTV_DESCRIPTOR_POOL_SIZE] = {nullptr};
    DescriptorAllocator *m_pDSVDescriptorAllocator = nullptr;

    UINT m_maxDescriptorCount = 0;
    UINT m_descriptorSize = 0;

    void CreateFence();
    void CleanupFence();
    void CreateCommandList();
    void CleanupCommandList();

    UINT64 Fence();
    void   WaitForFenceValue();

    void Cleanup();

  public:
    BOOL Initialize(ID3D12Device5 *pDevice, UINT maxDescriptorCount);

    // descriptor 개수에 맞는 리소스 반환
    BOOL AllocDescriptorTable(DESCRIPTOR_HANDLE *pOutDescriptor, UINT numDescriptors);
    BOOL AllocRTVDescriptorTable(DESCRIPTOR_HANDLE *pOutDescriptor, UINT numDescriptors = 1);
    BOOL AllocDSVDescriptorTable(DESCRIPTOR_HANDLE *pOutDescriptor);

    void DeallocDescriptorTable(DESCRIPTOR_HANDLE *pDescriptor);
    void DeallocRTVDescriptorTable(DESCRIPTOR_HANDLE *pOutDescriptor);
    void DeallocDSVDescriptorTable(DESCRIPTOR_HANDLE *pOutDescriptor);

    // Resource Manage
    HRESULT CreateVertexBuffer(ID3D12Resource **ppOutBuffer, D3D12_VERTEX_BUFFER_VIEW *pOutVertexBufferView,
                               UINT64 sizePerVertex, UINT numVertex, const void *pInitData);
    HRESULT CreateIndexBuffer(ID3D12Resource **ppOutBuffer, D3D12_INDEX_BUFFER_VIEW *pOutIndexBufferView,
                              UINT numIndices, const void *pInitData);
    HRESULT CreateConstantBuffer(ID3D12Resource **ppOutBuffer, UINT64 sizePerCB, const void *pInitData);
    void    UpdateTextureForWrite(ID3D12Resource *pDestTexResource, ID3D12Resource *pSrcTexResource);

    HRESULT CreateTextureFromMemory(ID3D12Resource **ppOutResource, UINT width, UINT height, DXGI_FORMAT format,
                                    const BYTE *pInitImage);

    BOOL CreateTextureFromDDS(ID3D12Resource **ppOutResource, D3D12_RESOURCE_DESC *pOutDesc, const wchar_t *filename);
    BOOL CreateTextureFromWIC(ID3D12Resource **ppOutResource, D3D12_RESOURCE_DESC *pOutDesc, const wchar_t *filename);

    BOOL CreateTexturePair(ID3D12Resource **ppOutResource, ID3D12Resource **ppOutUploadBuffer, UINT Width, UINT Height,
                           DXGI_FORMAT format);

    BOOL CreateTexture(ID3D12Resource **ppOutResource, UINT width, UINT height, DXGI_FORMAT format,
                       D3D12_RESOURCE_FLAGS flags);

    UINT GetDescriptorSize() const { return m_descriptorSize; }

    D3D12ResourceManager() = default;
    ~D3D12ResourceManager();
};