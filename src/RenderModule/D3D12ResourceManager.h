#pragma once

#include "RendererTypedef.h"
enum TEXTURE_FILE_FORMAT
{
    TEXTURE_FILE_FORMAT_M
};

class DescriptorAllocator;
class D3D12ResourceManager
{
    static const UINT DESCRIPTOR_COUNT_PER_RTV = 20;
    static const UINT DESCRIPTOR_COUNT_PER_DSV = 20;
    static const UINT DESCRIPTOR_POOL_SIZE = 6;

    ID3D12Device              *m_pD3DDevice = nullptr;
    ID3D12CommandQueue        *m_pCommandQueue = nullptr;
    ID3D12CommandAllocator    *m_pCommandAllocator = nullptr;
    ID3D12GraphicsCommandList *m_pCommandList = nullptr;

    HANDLE       m_hFenceEvent = nullptr;
    ID3D12Fence *m_pFence = nullptr;
    UINT64       m_ui64FenceValue = 0;

    DescriptorAllocator *m_pDescriptorAllocators[DESCRIPTOR_POOL_SIZE];
    DescriptorAllocator *m_pRTVDescriptorAllocator = nullptr;
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

    // descriptor ������ �´� ���ҽ� ��ȯ
    BOOL AllocDescriptorTable(DESCRIPTOR_HANDLE *pOutDescriptor, UINT numDescriptors);
    BOOL AllocRTVDescriptorTable(DESCRIPTOR_HANDLE *pOutDescriptor);
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

    HRESULT CreateDDSTextureFromFile(ID3D12Resource **ppOutResource, const wchar_t *filename, const bool isCubeMap);

    BOOL CreateTexturePair(ID3D12Resource **ppOutResource, ID3D12Resource **ppOutUploadBuffer, UINT Width, UINT Height,
                           DXGI_FORMAT format);

    BOOL CreateTexture(ID3D12Resource **ppOutResource, UINT width, UINT height, DXGI_FORMAT format,
                       D3D12_RESOURCE_FLAGS flags);

    UINT GetDescriptorSize() const { return m_descriptorSize; }

    D3D12ResourceManager() = default;
    ~D3D12ResourceManager();
};