#include "pch.h"

#include <DirectXTK/DDSTextureLoader.h>
#include <DirectXTK/WICTextureLoader.h>

#include "DescriptorAllocator.h"

#include "D3D12ResourceManager.h"

using namespace DirectX;
BOOL D3D12ResourceManager::Initialize(ID3D12Device5 *pDevice, UINT maxDescriptorCount)
{
    BOOL result = FALSE;

    m_pD3DDevice = pDevice;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    if (FAILED(m_pD3DDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue))))
    {
        __debugbreak();
        goto lb_return;
    }
    CreateCommandList();

    CreateFence();

    m_descriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    m_maxDescriptorCount = maxDescriptorCount;

    // Descriptor Pool Initialize
    m_pDescriptorAllocators[0] = new DescriptorAllocator;
    m_pDescriptorAllocators[0]->Initialize(pDevice, m_maxDescriptorCount, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_pDescriptorAllocators[1] = new DescriptorAllocator;
    m_pDescriptorAllocators[1]->Initialize(pDevice, m_maxDescriptorCount, 4, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_pDescriptorAllocators[2] = new DescriptorAllocator;
    m_pDescriptorAllocators[2]->Initialize(pDevice, m_maxDescriptorCount, 8, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_pDescriptorAllocators[3] = new DescriptorAllocator;
    m_pDescriptorAllocators[3]->Initialize(pDevice, m_maxDescriptorCount, 16, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_pDescriptorAllocators[4] = new DescriptorAllocator;
    m_pDescriptorAllocators[4]->Initialize(pDevice, m_maxDescriptorCount, 32, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_pDescriptorAllocators[5] = new DescriptorAllocator;
    m_pDescriptorAllocators[5]->Initialize(pDevice, m_maxDescriptorCount, 64, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    m_pRTVDescriptorAllocators[0] = new DescriptorAllocator;
    m_pRTVDescriptorAllocators[0]->Initialize(pDevice, DESCRIPTOR_COUNT_PER_RTV, 1, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_pRTVDescriptorAllocators[1] = new DescriptorAllocator;
    m_pRTVDescriptorAllocators[1]->Initialize(pDevice, DESCRIPTOR_COUNT_PER_RTV, 4, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_pRTVDescriptorAllocators[2] = new DescriptorAllocator;
    m_pRTVDescriptorAllocators[2]->Initialize(pDevice, DESCRIPTOR_COUNT_PER_RTV, 8, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_pRTVDescriptorAllocators[3] = new DescriptorAllocator;
    m_pRTVDescriptorAllocators[3]->Initialize(pDevice, DESCRIPTOR_COUNT_PER_RTV, 16, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    m_pDSVDescriptorAllocator = new DescriptorAllocator;
    m_pDSVDescriptorAllocator->Initialize(pDevice, DESCRIPTOR_COUNT_PER_DSV, 1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    result = TRUE;
lb_return:
    return result;
}

BOOL D3D12ResourceManager::AllocDescriptorTable(DESCRIPTOR_HANDLE *pOutDescriptor, UINT numDescriptors)
{
    assert(numDescriptors > 0);
    if (numDescriptors == 1)
        return m_pDescriptorAllocators[0]->Alloc(pOutDescriptor);
    if (numDescriptors <= 4)
        return m_pDescriptorAllocators[1]->Alloc(pOutDescriptor);
    if (numDescriptors <= 8)
        return m_pDescriptorAllocators[2]->Alloc(pOutDescriptor);
    if (numDescriptors <= 16)
        return m_pDescriptorAllocators[3]->Alloc(pOutDescriptor);
    if (numDescriptors <= 32)
        return m_pDescriptorAllocators[4]->Alloc(pOutDescriptor);
    if (numDescriptors <= 64)
        return m_pDescriptorAllocators[5]->Alloc(pOutDescriptor);
    return FALSE;
}

BOOL D3D12ResourceManager::AllocRTVDescriptorTable(DESCRIPTOR_HANDLE *pOutDescriptor, UINT numDescriptors)
{
    assert(numDescriptors > 0);
    if (numDescriptors == 1)
        return m_pRTVDescriptorAllocators[0]->Alloc(pOutDescriptor);
    if (numDescriptors <= 4)
        return m_pRTVDescriptorAllocators[1]->Alloc(pOutDescriptor);
    if (numDescriptors <= 8)
        return m_pRTVDescriptorAllocators[2]->Alloc(pOutDescriptor);
    if (numDescriptors <= 16)
        return m_pRTVDescriptorAllocators[3]->Alloc(pOutDescriptor);
#ifdef _DEBUG
    __debugbreak();
#endif
    return FALSE;
}

BOOL D3D12ResourceManager::AllocDSVDescriptorTable(DESCRIPTOR_HANDLE *pOutDescriptor)
{
    return m_pDSVDescriptorAllocator->Alloc(pOutDescriptor);
}

void D3D12ResourceManager::DeallocDescriptorTable(DESCRIPTOR_HANDLE *pDescriptor)
{
    switch (pDescriptor->descriptorCount)
    {
    case 1:
        m_pDescriptorAllocators[0]->DeAlloc(pDescriptor);
        break;
    case 4:
        m_pDescriptorAllocators[1]->DeAlloc(pDescriptor);
        break;
    case 8:
        m_pDescriptorAllocators[2]->DeAlloc(pDescriptor);
        break;
    case 16:
        m_pDescriptorAllocators[3]->DeAlloc(pDescriptor);
        break;
    case 32:
        m_pDescriptorAllocators[4]->DeAlloc(pDescriptor);
        break;
    case 64:
        m_pDescriptorAllocators[5]->DeAlloc(pDescriptor);
    default:
        __debugbreak();
        break;
    }
}
void D3D12ResourceManager::DeallocRTVDescriptorTable(DESCRIPTOR_HANDLE *pDescriptor)
{
    switch (pDescriptor->descriptorCount)
    {
    case 1:
        m_pRTVDescriptorAllocators[0]->DeAlloc(pDescriptor);
        break;
    case 4:
        m_pRTVDescriptorAllocators[1]->DeAlloc(pDescriptor);
        break;
    case 8:
        m_pRTVDescriptorAllocators[2]->DeAlloc(pDescriptor);
        break;
    case 16:
        m_pRTVDescriptorAllocators[3]->DeAlloc(pDescriptor);
        break;
    default:
        __debugbreak();
        break;
    }
}
void D3D12ResourceManager::DeallocDSVDescriptorTable(DESCRIPTOR_HANDLE *pOutDescriptor)
{
    m_pDSVDescriptorAllocator->DeAlloc(pOutDescriptor);
}
HRESULT
D3D12ResourceManager::CreateVertexBuffer(ID3D12Resource **ppOutBuffer, D3D12_VERTEX_BUFFER_VIEW *pOutVertexBufferView,
                                         UINT64 sizePerVertex, UINT numVertex, const void *pInitData)
{
    HRESULT hr = S_OK;

    D3D12_VERTEX_BUFFER_VIEW VertexBufferView = {};
    ID3D12Resource          *pVertexBuffer = nullptr;
    ID3D12Resource          *pUploadBuffer = nullptr;
    UINT                     VertexBufferSize = sizePerVertex * numVertex;

    // create vertexbuffer for rendering
    hr = m_pD3DDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
                                               &CD3DX12_RESOURCE_DESC::Buffer(VertexBufferSize),
                                               D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pVertexBuffer));

    if (FAILED(hr))
    {
        __debugbreak();
        goto lb_return;
    }
    if (pInitData)
    {
        if (FAILED(m_pCommandAllocator->Reset()))
            __debugbreak();

        if (FAILED(m_pCommandList->Reset(m_pCommandAllocator, nullptr)))
            __debugbreak();

        hr = m_pD3DDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(VertexBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&pUploadBuffer));

        if (FAILED(hr))
        {
            __debugbreak();
            goto lb_return;
        }

        // Copy the triangle data to the vertex buffer.
        UINT8        *pVertexDataBegin = nullptr;
        CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.

        hr = pUploadBuffer->Map(0, &readRange, reinterpret_cast<void **>(&pVertexDataBegin));
        if (FAILED(hr))
        {
            __debugbreak();
            goto lb_return;
        }
        memcpy(pVertexDataBegin, pInitData, VertexBufferSize);
        pUploadBuffer->Unmap(0, nullptr);

        m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pVertexBuffer,
                                                                                 D3D12_RESOURCE_STATE_COMMON,
                                                                                 D3D12_RESOURCE_STATE_COPY_DEST));
        m_pCommandList->CopyBufferRegion(pVertexBuffer, 0, pUploadBuffer, 0, VertexBufferSize);
        m_pCommandList->ResourceBarrier(
            1, &CD3DX12_RESOURCE_BARRIER::Transition(pVertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST,
                                                     D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

        m_pCommandList->Close();

        ID3D12CommandList *ppCommandLists[] = {m_pCommandList};
        m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        Fence();
        WaitForFenceValue();
    }

    // Initialize the vertex buffer view.
    VertexBufferView.BufferLocation = pVertexBuffer->GetGPUVirtualAddress();
    VertexBufferView.StrideInBytes = sizePerVertex;
    VertexBufferView.SizeInBytes = VertexBufferSize;

    pVertexBuffer->SetName(L"Vertex Buffer");

    *pOutVertexBufferView = VertexBufferView;
    *ppOutBuffer = pVertexBuffer;

lb_return:
    if (pUploadBuffer)
    {
        pUploadBuffer->Release();
        pUploadBuffer = nullptr;
    }
    return hr;
}

HRESULT D3D12ResourceManager::CreateIndexBuffer(ID3D12Resource         **ppOutBuffer,
                                                D3D12_INDEX_BUFFER_VIEW *pOutIndexBufferView, UINT numIndices,
                                                const void *pInitData)
{
    HRESULT hr = S_OK;

    D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
    ID3D12Resource         *pIndexBuffer = nullptr;
    ID3D12Resource         *pUploadBuffer = nullptr;
    UINT                    indexBufferSize = sizeof(UINT) * numIndices;

    // create vertexbuffer for rendering
    hr = m_pD3DDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
                                               &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
                                               D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pIndexBuffer));

    if (FAILED(hr))
    {
        __debugbreak();
        goto lb_return;
    }
    if (pInitData)
    {
        if (FAILED(m_pCommandAllocator->Reset()))
            __debugbreak();

        if (FAILED(m_pCommandList->Reset(m_pCommandAllocator, nullptr)))
            __debugbreak();

        hr = m_pD3DDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&pUploadBuffer));

        if (FAILED(hr))
        {
            __debugbreak();
            goto lb_return;
        }

        // Copy the index data to the index buffer.
        UINT8        *pIndexDataBegin = nullptr;
        CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.

        hr = pUploadBuffer->Map(0, &readRange, reinterpret_cast<void **>(&pIndexDataBegin));
        if (FAILED(hr))
        {
            __debugbreak();
            goto lb_return;
        }
        memcpy(pIndexDataBegin, pInitData, indexBufferSize);
        pUploadBuffer->Unmap(0, nullptr);

        m_pCommandList->ResourceBarrier(1,
                                        &CD3DX12_RESOURCE_BARRIER::Transition(pIndexBuffer, D3D12_RESOURCE_STATE_COMMON,
                                                                              D3D12_RESOURCE_STATE_COPY_DEST));
        m_pCommandList->CopyBufferRegion(pIndexBuffer, 0, pUploadBuffer, 0, indexBufferSize);
        m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pIndexBuffer,
                                                                                 D3D12_RESOURCE_STATE_COPY_DEST,
                                                                                 D3D12_RESOURCE_STATE_INDEX_BUFFER));

        m_pCommandList->Close();

        ID3D12CommandList *ppCommandLists[] = {m_pCommandList};
        m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        Fence();
        WaitForFenceValue();
    }

    // Initialize the vertex buffer view.
    indexBufferView.BufferLocation = pIndexBuffer->GetGPUVirtualAddress();
    indexBufferView.Format = DXGI_FORMAT_R32_UINT;
    indexBufferView.SizeInBytes = indexBufferSize;

    pIndexBuffer->SetName(L"Index Buffer");

    *pOutIndexBufferView = indexBufferView;
    *ppOutBuffer = pIndexBuffer;

lb_return:

    if (pUploadBuffer)
    {
        pUploadBuffer->Release();
        pUploadBuffer = nullptr;
    }
    return hr;
}

HRESULT D3D12ResourceManager::CreateConstantBuffer(ID3D12Resource **ppOutBuffer, UINT64 sizePerCB,
                                                   const void *pInitData)
{
    HRESULT hr = S_OK;

    ID3D12Resource *pConstantBuffer = nullptr;
    ID3D12Resource *pUploadBuffer = nullptr;
    UINT            constantBufferSize = sizePerCB;

    // create vertexbuffer for rendering
    hr = m_pD3DDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
                                               &CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize),
                                               D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pConstantBuffer));

    if (FAILED(hr))
    {
        __debugbreak();
        goto lb_return;
    }
    if (pInitData)
    {
        if (FAILED(m_pCommandAllocator->Reset()))
            __debugbreak();

        if (FAILED(m_pCommandList->Reset(m_pCommandAllocator, nullptr)))
            __debugbreak();

        hr = m_pD3DDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&pUploadBuffer));

        if (FAILED(hr))
        {
            __debugbreak();
            goto lb_return;
        }

        // Copy the index data to the index buffer.
        UINT8        *pConstantDataBegin = nullptr;
        CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.

        hr = pUploadBuffer->Map(0, &readRange, reinterpret_cast<void **>(&pConstantDataBegin));
        if (FAILED(hr))
        {
            __debugbreak();
            goto lb_return;
        }
        memcpy(pConstantDataBegin, pInitData, constantBufferSize);
        pUploadBuffer->Unmap(0, nullptr);

        m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pConstantBuffer,
                                                                                 D3D12_RESOURCE_STATE_COMMON,
                                                                                 D3D12_RESOURCE_STATE_COPY_DEST));
        m_pCommandList->CopyBufferRegion(pConstantBuffer, 0, pUploadBuffer, 0, constantBufferSize);
        m_pCommandList->ResourceBarrier(
            1, &CD3DX12_RESOURCE_BARRIER::Transition(pConstantBuffer, D3D12_RESOURCE_STATE_COPY_DEST,
                                                     D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

        m_pCommandList->Close();

        ID3D12CommandList *ppCommandLists[] = {m_pCommandList};
        m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        Fence();
        WaitForFenceValue();
    }

    *ppOutBuffer = pConstantBuffer;

lb_return:

    pConstantBuffer->SetName(L"Mesh Constant Buffer");

    if (pUploadBuffer)
    {
        pUploadBuffer->Release();
        pUploadBuffer = nullptr;
    }
    return hr;
}

void D3D12ResourceManager::UpdateTextureForWrite(ID3D12Resource *pDestTexResource, ID3D12Resource *pSrcTexResource)
{
    const DWORD                        MAX_SUB_RESOURCE_NUM = 32;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint[MAX_SUB_RESOURCE_NUM] = {};
    UINT                               Rows[MAX_SUB_RESOURCE_NUM] = {};
    UINT64                             RowSize[MAX_SUB_RESOURCE_NUM] = {};
    UINT64                             TotalBytes = 0;

    D3D12_RESOURCE_DESC Desc = pDestTexResource->GetDesc();
    if (Desc.MipLevels > (UINT)_countof(Footprint))
        __debugbreak();

    m_pD3DDevice->GetCopyableFootprints(&Desc, 0, Desc.MipLevels, 0, Footprint, Rows, RowSize, &TotalBytes);

    if (FAILED(m_pCommandAllocator->Reset()))
        __debugbreak();

    if (FAILED(m_pCommandList->Reset(m_pCommandAllocator, nullptr)))
        __debugbreak();

    m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pDestTexResource,
                                                                             D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
                                                                             D3D12_RESOURCE_STATE_COPY_DEST));
    for (DWORD i = 0; i < Desc.MipLevels; i++)
    {

        D3D12_TEXTURE_COPY_LOCATION destLocation = {};
        destLocation.PlacedFootprint = Footprint[i];
        destLocation.pResource = pDestTexResource;
        destLocation.SubresourceIndex = i;
        destLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

        D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
        srcLocation.PlacedFootprint = Footprint[i];
        srcLocation.pResource = pSrcTexResource;
        srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

        m_pCommandList->CopyTextureRegion(&destLocation, 0, 0, 0, &srcLocation, nullptr);
    }
    m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pDestTexResource,
                                                                             D3D12_RESOURCE_STATE_COPY_DEST,
                                                                             D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE));
    m_pCommandList->Close();

    ID3D12CommandList *ppCommandLists[] = {m_pCommandList};
    m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    Fence();
    WaitForFenceValue();
}

HRESULT D3D12ResourceManager::CreateTextureFromMemory(ID3D12Resource **ppOutResource, UINT width, UINT height,
                                                      DXGI_FORMAT format, const BYTE *pInitImage)
{
    ID3D12Resource *pTexResource = nullptr;
    ID3D12Resource *pUploadBuffer = nullptr;

    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = format; // ex) DXGI_FORMAT_R8G8B8A8_UNORM, etc...
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    if (FAILED(m_pD3DDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &textureDesc,
            D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&pTexResource))))
    {
        __debugbreak();
    }

    if (pInitImage)
    {
        D3D12_RESOURCE_DESC                Desc = pTexResource->GetDesc();
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint;
        UINT                               Rows = 0;
        UINT64                             RowSize = 0;
        UINT64                             TotalBytes = 0;

        m_pD3DDevice->GetCopyableFootprints(&Desc, 0, 1, 0, &Footprint, &Rows, &RowSize, &TotalBytes);

        BYTE         *pMappedPtr = nullptr;
        CD3DX12_RANGE writeRange(0, 0);

        UINT64 uploadBufferSize = GetRequiredIntermediateSize(pTexResource, 0, 1);

        if (FAILED(m_pD3DDevice->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                IID_PPV_ARGS(&pUploadBuffer))))
        {
            __debugbreak();
        }

        HRESULT hr = pUploadBuffer->Map(0, &writeRange, reinterpret_cast<void **>(&pMappedPtr));
        if (FAILED(hr))
            __debugbreak();

        const BYTE *pSrc = pInitImage;
        BYTE       *pDest = pMappedPtr;
        for (UINT y = 0; y < height; y++)
        {
            memcpy(pDest, pSrc, width * 4);
            pSrc += (width * 4);
            pDest += Footprint.Footprint.RowPitch;
        }
        // Unmap
        pUploadBuffer->Unmap(0, nullptr);

        UpdateTextureForWrite(pTexResource, pUploadBuffer);

        pUploadBuffer->Release();
        pUploadBuffer = nullptr;
    }

    pTexResource->SetName(L"Texture");

    *ppOutResource = pTexResource;

    return TRUE;
}

BOOL D3D12ResourceManager::CreateTextureFromDDS(ID3D12Resource **ppOutResource, D3D12_RESOURCE_DESC *pOutDesc,
                                                const wchar_t *filename)
{
    BOOL result = FALSE;

    ID3D12Resource *pTexResource = nullptr;
    ID3D12Resource *pUploadBuffer = nullptr;

    D3D12_RESOURCE_DESC textureDesc = {};

    std::unique_ptr<uint8_t[]>          ddsData;
    std::vector<D3D12_SUBRESOURCE_DATA> subresourceData;
    if (FAILED(LoadDDSTextureFromFile(m_pD3DDevice, filename, &pTexResource, ddsData, subresourceData)))
    {
#ifdef _DEBUG
        __debugbreak();
#endif // _DEBUG

        goto lb_return;
    }
    textureDesc = pTexResource->GetDesc();
    UINT   subresoucesize = (UINT)subresourceData.size();
    UINT64 uploadBufferSize = GetRequiredIntermediateSize(pTexResource, 0, subresoucesize);

    // Create the GPU upload buffer.
    if (FAILED(m_pD3DDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&pUploadBuffer))))
    {
        __debugbreak();
    }

    if (FAILED(m_pCommandAllocator->Reset()))
        __debugbreak();

    if (FAILED(m_pCommandList->Reset(m_pCommandAllocator, nullptr)))
        __debugbreak();

    m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pTexResource,
                                                                             D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
                                                                             D3D12_RESOURCE_STATE_COPY_DEST));
    UpdateSubresources(m_pCommandList, pTexResource, pUploadBuffer, 0, 0, subresoucesize, &subresourceData[0]);
    m_pCommandList->ResourceBarrier(1,
                                    &CD3DX12_RESOURCE_BARRIER::Transition(pTexResource, D3D12_RESOURCE_STATE_COPY_DEST,
                                                                          D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE));

    m_pCommandList->Close();

    ID3D12CommandList *ppCommandLists[] = {m_pCommandList};
    m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    Fence();
    WaitForFenceValue();

    if (pUploadBuffer)
    {
        pUploadBuffer->Release();
        pUploadBuffer = nullptr;
    }
    *ppOutResource = pTexResource;
    *pOutDesc = textureDesc;
    result = TRUE;
lb_return:
    return result;
}

BOOL D3D12ResourceManager::CreateTextureFromWIC(ID3D12Resource **ppOutResource, D3D12_RESOURCE_DESC *pOutDesc,
                                                const wchar_t *filename)
{
    BOOL result = FALSE;

    ID3D12Resource *pTexResource = nullptr;
    ID3D12Resource *pUploadBuffer = nullptr;

    D3D12_RESOURCE_DESC textureDesc = {};

    std::unique_ptr<uint8_t[]> wicData;
    D3D12_SUBRESOURCE_DATA     subresourceData;
    if (FAILED(LoadWICTextureFromFile(m_pD3DDevice, filename, &pTexResource, wicData, subresourceData)))
    {
        goto lb_return;
    }
    textureDesc = pTexResource->GetDesc();
    UINT   subresoucesize = 1; // WIC는 subresource 개수가 1개
    UINT64 uploadBufferSize = GetRequiredIntermediateSize(pTexResource, 0, subresoucesize);

    // Create the GPU upload buffer.
    if (FAILED(m_pD3DDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&pUploadBuffer))))
    {
        __debugbreak();
    }

    if (FAILED(m_pCommandAllocator->Reset()))
        __debugbreak();

    if (FAILED(m_pCommandList->Reset(m_pCommandAllocator, nullptr)))
        __debugbreak();

    m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pTexResource,
                                                                             D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
                                                                             D3D12_RESOURCE_STATE_COPY_DEST));
    UpdateSubresources(m_pCommandList, pTexResource, pUploadBuffer, 0, 0, subresoucesize, &subresourceData);
    m_pCommandList->ResourceBarrier(1,
                                    &CD3DX12_RESOURCE_BARRIER::Transition(pTexResource, D3D12_RESOURCE_STATE_COPY_DEST,
                                                                          D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE));

    m_pCommandList->Close();

    ID3D12CommandList *ppCommandLists[] = {m_pCommandList};
    m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    Fence();
    WaitForFenceValue();

    if (pUploadBuffer)
    {
        pUploadBuffer->Release();
        pUploadBuffer = nullptr;
    }
    *ppOutResource = pTexResource;
    *pOutDesc = textureDesc;
    result = TRUE;
lb_return:
    return result;
}

BOOL D3D12ResourceManager::CreateTexturePair(ID3D12Resource **ppOutResource, ID3D12Resource **ppOutUploadBuffer,
                                             UINT Width, UINT Height, DXGI_FORMAT format)
{
    ID3D12Resource *pTexResource = nullptr;
    ID3D12Resource *pUploadBuffer = nullptr;

    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = format;
    textureDesc.Width = Width;
    textureDesc.Height = Height;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 1;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    if (FAILED(m_pD3DDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &textureDesc,
            D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&pTexResource))))
    {
        __debugbreak();
    }

    UINT64 uploadBufferSize = GetRequiredIntermediateSize(pTexResource, 0, 1);

    if (FAILED(m_pD3DDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&pUploadBuffer))))
    {
        __debugbreak();
    }
    *ppOutResource = pTexResource;
    *ppOutUploadBuffer = pUploadBuffer;

    return TRUE;
}

BOOL D3D12ResourceManager::CreateTexture(ID3D12Resource **ppOutResource, UINT width, UINT height, DXGI_FORMAT format,
                                         D3D12_RESOURCE_FLAGS flags)
{
    ID3D12Resource *pTexResource = nullptr;

    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = format;
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.Flags = flags;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    if (FAILED(m_pD3DDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                     D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_COMMON,
                                                     nullptr, IID_PPV_ARGS(&pTexResource))))
    {
        __debugbreak();
    }

    *ppOutResource = pTexResource;
    return TRUE;
}

D3D12ResourceManager::~D3D12ResourceManager() { Cleanup(); }

void D3D12ResourceManager::CreateFence()
{
    // Create synchronization objects and wait until assets have been uploaded to
    // the GPU.
    if (FAILED(m_pD3DDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence))))
    {
        __debugbreak();
    }

    m_ui64FenceValue = 0;

    // Create an event handle to use for frame synchronization.
    m_hFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

void D3D12ResourceManager::CleanupFence()
{
    if (m_hFenceEvent)
    {
        CloseHandle(m_hFenceEvent);
        m_hFenceEvent = nullptr;
    }
    if (m_pFence)
    {
        m_pFence->Release();
        m_pFence = nullptr;
    }
}

void D3D12ResourceManager::CreateCommandList()
{
    if (FAILED(
            m_pD3DDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocator))))
    {
        __debugbreak();
    }

    // Create the command list.
    if (FAILED(m_pD3DDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator, nullptr,
                                               IID_PPV_ARGS(&m_pCommandList))))
    {
        __debugbreak();
    }

    // Command lists are created in the recording state, but there is nothing
    // to record yet. The main loop expects it to be closed, so close it now.
    m_pCommandList->Close();
}

void D3D12ResourceManager::CleanupCommandList()
{
    if (m_pCommandList)
    {
        m_pCommandList->Release();
        m_pCommandList = nullptr;
    }
    if (m_pCommandAllocator)
    {
        m_pCommandAllocator->Release();
        m_pCommandAllocator = nullptr;
    }
}

UINT64 D3D12ResourceManager::Fence()
{
    m_ui64FenceValue++;
    m_pCommandQueue->Signal(m_pFence, m_ui64FenceValue);
    return m_ui64FenceValue;
}

void D3D12ResourceManager::WaitForFenceValue()
{
    const UINT64 ExpectedFenceValue = m_ui64FenceValue;

    // Wait until the previous frame is finished.
    if (m_pFence->GetCompletedValue() < ExpectedFenceValue)
    {
        m_pFence->SetEventOnCompletion(ExpectedFenceValue, m_hFenceEvent);
        WaitForSingleObject(m_hFenceEvent, INFINITE);
    }
}

void D3D12ResourceManager::Cleanup()
{
    WaitForFenceValue();

    if (m_pCommandQueue)
    {
        m_pCommandQueue->Release();
        m_pCommandQueue = nullptr;
    }

    CleanupCommandList();

    CleanupFence();

    // Pool
    for (UINT i = 0; i < DESCRIPTOR_POOL_SIZE; i++)
    {
        if (m_pDescriptorAllocators[i])
        {
            delete m_pDescriptorAllocators[i];
            m_pDescriptorAllocators[i] = nullptr;
        }
    }
    for (UINT i = 0; i < RTV_DESCRIPTOR_POOL_SIZE; i++)
    {
        if (m_pRTVDescriptorAllocators[i])
        {
            delete m_pRTVDescriptorAllocators[i];
            m_pRTVDescriptorAllocators[i] = nullptr;
        }
    }
    if (m_pDSVDescriptorAllocator)
    {
        delete m_pDSVDescriptorAllocator;
        m_pDSVDescriptorAllocator = nullptr;
    }
}
