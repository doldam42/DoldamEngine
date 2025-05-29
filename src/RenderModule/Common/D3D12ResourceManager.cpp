#include "pch.h"

#include <d3d12.h>
#include <d3dx12.h>

#include <DDSTextureLoader/DDSTextureLoader12.h>
#include <DirectXTex/DirectXTex.h>
#include <WICTextureLoader/WICTextureLoader12.h>

#ifdef RENDERER_RAYTRACING
#include "../RendererRaytracing/D3D12Renderer.h"
#elif defined(RENDERER_D3D12)
#include "../RendererD3D12/D3D12Renderer.h"
#endif

#include "CommandListPool.h"

#include "D3DResourceRecycleBin.h"
#include "DescriptorAllocator.h"

#include "D3D12ResourceManager.h"

using namespace DirectX;
BOOL D3D12ResourceManager::Initialize(D3D12Renderer *pRenderer, ID3D12CommandQueue *pCmdQueue, UINT maxDescriptorCount)
{
    BOOL result = FALSE;

    m_pRenderer = pRenderer;
    m_pD3DDevice = pRenderer->GetD3DDevice();
    m_pCommandQueue = pCmdQueue;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    m_pResourceBinDefault = new D3DResourceRecycleBin;
    m_pResourceBinDefault->Initialize(m_pD3DDevice, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_FLAG_NONE,
                                      D3D12_RESOURCE_STATE_COMMON, L"Common Resource");

    m_pResourceBinUpload = new D3DResourceRecycleBin;
    m_pResourceBinUpload->Initialize(m_pD3DDevice, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_FLAG_NONE,
                                     D3D12_RESOURCE_STATE_GENERIC_READ, L"Upload Buffer");

    m_descriptorSize = m_pD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    m_maxDescriptorCount = maxDescriptorCount;

    result = TRUE;
lb_return:
    return result;
}

BOOL D3D12ResourceManager::AllocDescriptorTable(DESCRIPTOR_HANDLE *pOutDescriptor, UINT numDescriptors)
{
    assert(numDescriptors > 0);
    DescriptorAllocator *pAllocator = FindAllocator(numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    if (!pAllocator)
    {
        return FALSE;
    }
    return pAllocator->Alloc(pOutDescriptor);
}

BOOL D3D12ResourceManager::AllocRTVDescriptorTable(DESCRIPTOR_HANDLE *pOutDescriptor, UINT numDescriptors)
{
    assert(numDescriptors > 0);
    DescriptorAllocator *pAllocator = FindAllocator(numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    if (!pAllocator)
    {
        return FALSE;
    }
    return pAllocator->Alloc(pOutDescriptor);
}

BOOL D3D12ResourceManager::AllocDSVDescriptorTable(DESCRIPTOR_HANDLE *pOutDescriptor, UINT numDescriptors)
{
    assert(numDescriptors > 0);
    DescriptorAllocator *pAllocator = FindAllocator(numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    if (!pAllocator)
    {
        return FALSE;
    }
    return pAllocator->Alloc(pOutDescriptor);
}

void D3D12ResourceManager::DeallocDescriptorTable(DESCRIPTOR_HANDLE *pDescriptor)
{
    DescriptorAllocator *pAllocator =
        FindAllocator(pDescriptor->descriptorCount, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    DASSERT(pAllocator);
    pAllocator->DeAlloc(pDescriptor);
}
void D3D12ResourceManager::DeallocRTVDescriptorTable(DESCRIPTOR_HANDLE *pDescriptor)
{
    DescriptorAllocator *pAllocator = FindAllocator(pDescriptor->descriptorCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    DASSERT(pAllocator);
    pAllocator->DeAlloc(pDescriptor);
}
void D3D12ResourceManager::DeallocDSVDescriptorTable(DESCRIPTOR_HANDLE *pDescriptor)
{
    DescriptorAllocator *pAllocator = FindAllocator(pDescriptor->descriptorCount, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    DASSERT(pAllocator);
    pAllocator->DeAlloc(pDescriptor);
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

    pVertexBuffer = m_pResourceBinDefault->Alloc(VertexBufferSize);

    if (pInitData)
    {
        CommandListPool            *pCommandListPool = m_pRenderer->GetCommandListPool(0);
        ID3D12GraphicsCommandList4 *pCommandList = pCommandListPool->GetCurrentCommandList();

        pUploadBuffer = m_pResourceBinUpload->Alloc(VertexBufferSize);

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

        pCommandList->ResourceBarrier(1,
                                      &CD3DX12_RESOURCE_BARRIER::Transition(pVertexBuffer, D3D12_RESOURCE_STATE_COMMON,
                                                                            D3D12_RESOURCE_STATE_COPY_DEST));
        pCommandList->CopyBufferRegion(pVertexBuffer, 0, pUploadBuffer, 0, VertexBufferSize);
        pCommandList->ResourceBarrier(
            1, &CD3DX12_RESOURCE_BARRIER::Transition(pVertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST,
                                                     D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

        pCommandListPool->CloseAndExecute(m_pCommandQueue);
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
        m_pResourceBinUpload->Free(pUploadBuffer, MAX_PENDING_FRAME_COUNT);
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
    pIndexBuffer = m_pResourceBinDefault->Alloc(indexBufferSize);

    if (pInitData)
    {
        CommandListPool            *pCommandListPool = m_pRenderer->GetCommandListPool(0);
        ID3D12GraphicsCommandList4 *pCommandList = pCommandListPool->GetCurrentCommandList();

        pUploadBuffer = m_pResourceBinUpload->Alloc(indexBufferSize);

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

        pCommandList->ResourceBarrier(1,
                                      &CD3DX12_RESOURCE_BARRIER::Transition(pIndexBuffer, D3D12_RESOURCE_STATE_COMMON,
                                                                            D3D12_RESOURCE_STATE_COPY_DEST));
        pCommandList->CopyBufferRegion(pIndexBuffer, 0, pUploadBuffer, 0, indexBufferSize);
        pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pIndexBuffer,
                                                                               D3D12_RESOURCE_STATE_COPY_DEST,
                                                                               D3D12_RESOURCE_STATE_INDEX_BUFFER));
        pCommandListPool->CloseAndExecute(m_pCommandQueue);
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
        m_pResourceBinUpload->Free(pUploadBuffer, MAX_PENDING_FRAME_COUNT);
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
    pConstantBuffer = m_pResourceBinDefault->Alloc(constantBufferSize);

    if (pInitData)
    {
        CommandListPool            *pCommandListPool = m_pRenderer->GetCommandListPool(0);
        ID3D12GraphicsCommandList4 *pCommandList = pCommandListPool->GetCurrentCommandList();

        pUploadBuffer = m_pResourceBinUpload->Alloc(constantBufferSize);

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

        pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pConstantBuffer,
                                                                               D3D12_RESOURCE_STATE_COMMON,
                                                                               D3D12_RESOURCE_STATE_COPY_DEST));
        pCommandList->CopyBufferRegion(pConstantBuffer, 0, pUploadBuffer, 0, constantBufferSize);
        pCommandList->ResourceBarrier(
            1, &CD3DX12_RESOURCE_BARRIER::Transition(pConstantBuffer, D3D12_RESOURCE_STATE_COPY_DEST,
                                                     D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
        pCommandListPool->CloseAndExecute(m_pCommandQueue);
    }

    *ppOutBuffer = pConstantBuffer;

lb_return:
    if (pUploadBuffer)
    {
        m_pResourceBinDefault->Free(pUploadBuffer, MAX_PENDING_FRAME_COUNT);
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

    CommandListPool            *pCommandListPool = m_pRenderer->GetCommandListPool(0);
    ID3D12GraphicsCommandList4 *pCommandList = pCommandListPool->GetCurrentCommandList();

    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pDestTexResource,
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

        pCommandList->CopyTextureRegion(&destLocation, 0, 0, 0, &srcLocation, nullptr);
    }
    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pDestTexResource,
                                                                           D3D12_RESOURCE_STATE_COPY_DEST,
                                                                           D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE));
    pCommandListPool->CloseAndExecute(m_pCommandQueue);
}

void D3D12ResourceManager::UpdateTextureWithImage(ID3D12Resource *pDestTexResource, const BYTE *pSrcBits, UINT srcWidth,
                                                  UINT srcHeight)
{
    D3D12_RESOURCE_DESC desc = pDestTexResource->GetDesc();
    if (srcWidth > desc.Width)
    {
        __debugbreak();
    }
    if (srcHeight > desc.Height)
    {
        __debugbreak();
    }

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint;

    UINT   Rows = 0;
    UINT64 RowSize = 0;
    UINT64 TotalBytes = 0;

    m_pD3DDevice->GetCopyableFootprints(&desc, 0, 1, 0, &Footprint, &Rows, &RowSize, &TotalBytes);

    BYTE         *pMappedPtr = nullptr;
    CD3DX12_RANGE writeRange(0, 0);
    ID3D12Resource *pUploadBuffer = m_pResourceBinUpload->Alloc(TotalBytes);
    HRESULT hr = pUploadBuffer->Map(0, &writeRange, reinterpret_cast<void **>(&pMappedPtr));
    if (FAILED(hr))
        __debugbreak();

    const BYTE *pSrc = pSrcBits;
    BYTE       *pDest = pMappedPtr;
    for (UINT y = 0; y < srcHeight; y++)
    {
        memcpy(pDest, pSrc, srcWidth * 4);
        pSrc += (srcWidth * 4);
        pDest += Footprint.Footprint.RowPitch;
    }
    
    CommandListPool            *pCommandListPool = m_pRenderer->GetCommandListPool(0);
    ID3D12GraphicsCommandList4 *pCommandList = pCommandListPool->GetCurrentCommandList();
    UpdateTexture(m_pD3DDevice, pCommandList, pDestTexResource, pUploadBuffer);
    pCommandListPool->CloseAndExecute(m_pCommandQueue);

    m_pResourceBinUpload->Free(pUploadBuffer, MAX_PENDING_FRAME_COUNT);
}

void D3D12ResourceManager::UpdateBuffer(ID3D12Resource *pDestBuffer, const void *pData, UINT sizeInBytes)
{
    HRESULT hr = S_OK;

    DASSERT(pData);

    ID3D12Resource *pUploadBuffer = m_pResourceBinUpload->Alloc(sizeInBytes);

    // Copy the index data to the index buffer.
    UINT8        *pDataBegin = nullptr;
    CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.

    hr = pUploadBuffer->Map(0, &readRange, reinterpret_cast<void **>(&pDataBegin));

    DASSERT(SUCCEEDED(hr));

    memcpy(pDataBegin, pData, sizeInBytes);
    pUploadBuffer->Unmap(0, nullptr);

    CommandListPool            *pCommandListPool = m_pRenderer->GetCommandListPool(0);
    ID3D12GraphicsCommandList4 *pCommandList = pCommandListPool->GetCurrentCommandList();
    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pDestBuffer, D3D12_RESOURCE_STATE_COMMON,
                                                                           D3D12_RESOURCE_STATE_COPY_DEST));
    pCommandList->CopyBufferRegion(pDestBuffer, 0, pUploadBuffer, 0, sizeInBytes);
    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pDestBuffer, D3D12_RESOURCE_STATE_COPY_DEST,
                                                                           D3D12_RESOURCE_STATE_COMMON));
    pCommandListPool->CloseAndExecute(m_pCommandQueue);

    if (pUploadBuffer)
    {
        m_pResourceBinUpload->Free(pUploadBuffer, MAX_PENDING_FRAME_COUNT);
        pUploadBuffer = nullptr;
    }
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

        pUploadBuffer = m_pResourceBinUpload->Alloc(uploadBufferSize);

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

        m_pResourceBinUpload->Free(pUploadBuffer, MAX_PENDING_FRAME_COUNT);
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

    pUploadBuffer = m_pResourceBinUpload->Alloc(uploadBufferSize);

    CommandListPool            *pCommandListPool = m_pRenderer->GetCommandListPool(0);
    ID3D12GraphicsCommandList4 *pCommandList = pCommandListPool->GetCurrentCommandList();
    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pTexResource,
                                                                           D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
                                                                           D3D12_RESOURCE_STATE_COPY_DEST));
    UpdateSubresources(pCommandList, pTexResource, pUploadBuffer, 0, 0, subresoucesize, &subresourceData[0]);
    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pTexResource, D3D12_RESOURCE_STATE_COPY_DEST,
                                                                           D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE));
    pCommandListPool->CloseAndExecute(m_pCommandQueue);

    if (pUploadBuffer)
    {
        m_pResourceBinUpload->Free(pUploadBuffer, MAX_PENDING_FRAME_COUNT);
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
    pUploadBuffer = m_pResourceBinUpload->Alloc(uploadBufferSize);

    CommandListPool            *pCommandListPool = m_pRenderer->GetCommandListPool(0);
    ID3D12GraphicsCommandList4 *pCommandList = pCommandListPool->GetCurrentCommandList();
    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pTexResource,
                                                                           D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
                                                                           D3D12_RESOURCE_STATE_COPY_DEST));
    UpdateSubresources(pCommandList, pTexResource, pUploadBuffer, 0, 0, subresoucesize, &subresourceData);
    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pTexResource, D3D12_RESOURCE_STATE_COPY_DEST,
                                                                           D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE));
    pCommandListPool->CloseAndExecute(m_pCommandQueue);

    if (pUploadBuffer)
    {
        m_pResourceBinUpload->Free(pUploadBuffer, MAX_PENDING_FRAME_COUNT);
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

void D3D12ResourceManager::Cleanup()
{
    if (m_pResourceBinDefault)
    {
        delete m_pResourceBinDefault;
        m_pResourceBinDefault = nullptr;
    }
    if (m_pResourceBinUpload)
    {
        delete m_pResourceBinUpload;
        m_pResourceBinUpload = nullptr;
    }

    // Pool
    for (UINT i = 0; i < DESCRIPTOR_POOL_SIZE; i++)
    {
        if (m_pDescriptorAllocators[i])
        {
            delete m_pDescriptorAllocators[i];
            m_pDescriptorAllocators[i] = nullptr;
        }
    }
    for (UINT i = 0; i < DESCRIPTOR_POOL_SIZE; i++)
    {
        if (m_pRTVDescriptorAllocators[i])
        {
            delete m_pRTVDescriptorAllocators[i];
            m_pRTVDescriptorAllocators[i] = nullptr;
        }
    }
    for (UINT i = 0; i < DESCRIPTOR_POOL_SIZE; i++)
    {
        if (m_pDSVDescriptorAllocators[i])
        {
            delete m_pDSVDescriptorAllocators[i];
            m_pDSVDescriptorAllocators[i] = nullptr;
        }
    }
}

DescriptorAllocator *D3D12ResourceManager::FindAllocator(UINT size, D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    DescriptorAllocator **ppDescriptorAllocators = nullptr;

    UINT descriptorCount;
    switch (type)
    {
    case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
        ppDescriptorAllocators = m_pDescriptorAllocators;
        descriptorCount = m_maxDescriptorCount;
        break;
    case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
        ppDescriptorAllocators = m_pRTVDescriptorAllocators;
        descriptorCount = DESCRIPTOR_COUNT_PER_RTV;
        break;
    case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
        ppDescriptorAllocators = m_pDSVDescriptorAllocators;
        descriptorCount = DESCRIPTOR_COUNT_PER_DSV;
        break;
    default:
        __debugbreak();
        break;
    }

    DescriptorAllocator *pSelectedAllocator = nullptr;
    UINT                 maxSize = 1;
    UINT                 i = 0;
    for (i = 0; i < DESCRIPTOR_POOL_SIZE; i++)
    {
        if (size <= maxSize)
        {
            pSelectedAllocator = ppDescriptorAllocators[i];
            break;
        }
        maxSize *= 2;
    }

    if (i == DESCRIPTOR_POOL_SIZE)
    {
        DASSERT(FALSE);
        return nullptr;
    }

    if (!pSelectedAllocator)
    {
        pSelectedAllocator = new DescriptorAllocator;
        pSelectedAllocator->Initialize(m_pD3DDevice, descriptorCount, maxSize, type);
        ppDescriptorAllocators[i] = pSelectedAllocator;
    }

    return pSelectedAllocator;
}

void D3D12ResourceManager::UpdateManagedResource()
{
    ULONGLONG CurTick = GetTickCount64();
    m_pResourceBinDefault->Update(CurTick);
    m_pResourceBinUpload->Update(CurTick);
}