#pragma once

#include "RendererTypedef.h"
enum TEXTURE_FILE_FORMAT
{
    TEXTURE_FILE_FORMAT_M
};

class D3DResourceRecycleBin;
class DescriptorAllocator;
class D3D12Renderer;
class D3D12ResourceManager
{
    static const UINT DESCRIPTOR_COUNT_PER_RTV = 32;
    static const UINT DESCRIPTOR_COUNT_PER_DSV = 32;
    static const UINT DESCRIPTOR_POOL_SIZE = 16;
    static const UINT RTV_DESCRIPTOR_POOL_SIZE = 4;

    D3D12Renderer *m_pRenderer = nullptr;
    ID3D12Device5 *m_pD3DDevice = nullptr;
    ID3D12CommandQueue  *m_pCommandQueue = nullptr;
    DescriptorAllocator *m_pDescriptorAllocators[DESCRIPTOR_POOL_SIZE] = {nullptr};
    DescriptorAllocator *m_pRTVDescriptorAllocators[DESCRIPTOR_POOL_SIZE] = {nullptr};
    DescriptorAllocator *m_pDSVDescriptorAllocators[DESCRIPTOR_POOL_SIZE] = {nullptr};

    D3DResourceRecycleBin *m_pResourceBinUpload = nullptr;
    D3DResourceRecycleBin *m_pResourceBinDefault = nullptr;

    UINT m_maxDescriptorCount = 0;
    UINT m_descriptorSize = 0;

    void Cleanup();

    DescriptorAllocator *FindAllocator(UINT size, D3D12_DESCRIPTOR_HEAP_TYPE type);

  public:
    BOOL Initialize(D3D12Renderer *pRenderer, ID3D12CommandQueue* pCmdQueue, UINT maxDescriptorCount);

    // descriptor 개수에 맞는 리소스 반환
    BOOL AllocDescriptorTable(DESCRIPTOR_HANDLE *pOutDescriptor, UINT numDescriptors);
    BOOL AllocRTVDescriptorTable(DESCRIPTOR_HANDLE *pOutDescriptor, UINT numDescriptors = 1);
    BOOL AllocDSVDescriptorTable(DESCRIPTOR_HANDLE *pOutDescriptor, UINT numDescriptors = 1);

    void DeallocDescriptorTable(DESCRIPTOR_HANDLE *pDescriptor);
    void DeallocRTVDescriptorTable(DESCRIPTOR_HANDLE *pDescriptor);
    void DeallocDSVDescriptorTable(DESCRIPTOR_HANDLE *pDescriptor);

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

    void UpdateManagedResource();

    UINT GetDescriptorSize() const { return m_descriptorSize; }

    D3D12ResourceManager() = default;
    ~D3D12ResourceManager();
};