#include "pch.h"

#ifdef RENDERER_RAYTRACING
#include "../RendererRaytracing/D3D12Renderer.h"
#include "../RendererRaytracing/GraphicsCommon.h"
#elif defined(RENDERER_D3D12)
#include "../RendererD3D12/D3D12Renderer.h"
#include "../RendererD3D12/GraphicsCommon.h"
#endif

#include "DescriptorAllocator.h"
#include "DescriptorPool.h"
#include "D3D12ResourceManager.h"
#include "D3DResourceRecycleBin.h"

#include "TransparencyManager.h"

void TransparencyManager::CreatDescriptorTable() 
{ 
    D3D12ResourceManager *pResourceManager = m_pRenderer->GetResourceManager();

    pResourceManager->AllocDescriptorTable(&m_descriptorTable, OIT_DESCRIPTOR_COUNT);
}

void TransparencyManager::CreateUAVCounterClearResource() 
{
    if (FAILED(m_pD3DDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT), D3D12_RESOURCE_FLAG_NONE), D3D12_RESOURCE_STATE_COPY_SOURCE,
            nullptr, IID_PPV_ARGS(&m_pUAVCounterClearResource))))
    {
        __debugbreak();
    }

    UINT *pSysMem = nullptr;
    HRESULT hr = m_pUAVCounterClearResource->Map(0, nullptr, reinterpret_cast<void **>(&pSysMem));
    if (SUCCEEDED(hr))
    {
        UINT clearValue = 0;
        memcpy(pSysMem, &clearValue, sizeof(UINT));
        m_pUAVCounterClearResource->Unmap(0, nullptr);
        pSysMem = nullptr;
    }
}

void TransparencyManager::CreateBuffers(UINT width, UINT height)
{
    ID3D12Resource *pFragmentListNodeFirstAddress = nullptr;
    ID3D12Resource *pFragmentListNode = nullptr;

    if (FAILED(m_pD3DDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32_UINT, width, height, 1, 1, 1, 0,
                                          D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
            D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pFragmentListNodeFirstAddress))))
    {
        __debugbreak();
    }

    UINT maxFragmentListbufferSizeInByte = m_maxFragmentListNodeCount * sizeof(FragmentListNode); // 192MB
    pFragmentListNode = m_pFragmentListRecycleBin->Alloc(maxFragmentListbufferSizeInByte);
    /*if (FAILED(m_pD3DDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(maxFragmentListbufferSizeInByte + sizeof(UINT),
                                           D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
            D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pFragmentListNode))))
    {
        __debugbreak();
    }*/

    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_descriptorTable.cpuHandle);

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Format = DXGI_FORMAT_R32_UINT;
    uavDesc.Texture2D.MipSlice = 0;
    uavDesc.Texture2D.PlaneSlice = 0;
    m_pD3DDevice->CreateUnorderedAccessView(pFragmentListNodeFirstAddress, nullptr, &uavDesc, cpuHandle);
    cpuHandle.Offset(m_srvDescriptorSize);

    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.Buffer.CounterOffsetInBytes = maxFragmentListbufferSizeInByte;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
    uavDesc.Buffer.NumElements = m_maxFragmentListNodeCount;
    uavDesc.Buffer.StructureByteStride = sizeof(FragmentListNode);

    m_pD3DDevice->CreateUnorderedAccessView(pFragmentListNode, pFragmentListNode, &uavDesc, cpuHandle);
    cpuHandle.Offset(m_srvDescriptorSize);

    m_pD3DDevice->CreateShaderResourceView(pFragmentListNodeFirstAddress, nullptr, cpuHandle);
    cpuHandle.Offset(m_srvDescriptorSize);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    srvDesc.Buffer.NumElements = m_maxFragmentListNodeCount;
    srvDesc.Buffer.StructureByteStride = sizeof(FragmentListNode);
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    m_pD3DDevice->CreateShaderResourceView(pFragmentListNode, &srvDesc, cpuHandle);
    cpuHandle.Offset(m_srvDescriptorSize);

    m_pFragmentList = pFragmentListNode;
    m_pFragmentListFirstNodeAddress = pFragmentListNodeFirstAddress;
    m_allocatedNodeCount = 0;
}

void TransparencyManager::CleanupBuffers()
{ 
    if (m_pFragmentList)
    {   
        m_pFragmentListRecycleBin->Free(m_pFragmentList, MAX_PENDING_FRAME_COUNT);
        m_pFragmentList = nullptr;
    }
    if (m_pFragmentListFirstNodeAddress)
    {   
        m_pFragmentListFirstNodeAddress->Release();
        m_pFragmentListFirstNodeAddress = nullptr;
    }
}

void TransparencyManager::CopyUAVCounterForRead(ID3D12GraphicsCommandList *pCommandList)
{
    ID3D12Resource *pReadbackBuffer = m_pReadbackBuffers[m_curContextIndex];

    CD3DX12_RESOURCE_BARRIER barriers[] = {
        CD3DX12_RESOURCE_BARRIER::Transition(m_pFragmentList, D3D12_RESOURCE_STATE_COMMON,
                                             D3D12_RESOURCE_STATE_COPY_SOURCE),
        CD3DX12_RESOURCE_BARRIER::Transition(pReadbackBuffer, D3D12_RESOURCE_STATE_COMMON,
                                             D3D12_RESOURCE_STATE_COPY_DEST)};

    pCommandList->ResourceBarrier(_countof(barriers), barriers);

    pCommandList->CopyBufferRegion(pReadbackBuffer, 0, m_pFragmentList,
                                   m_maxFragmentListNodeCount * sizeof(FragmentListNode), sizeof(UINT));

    barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(m_pFragmentList, D3D12_RESOURCE_STATE_COPY_SOURCE,
                                                       D3D12_RESOURCE_STATE_COMMON);
    barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(pReadbackBuffer, D3D12_RESOURCE_STATE_COPY_DEST,
                                                       D3D12_RESOURCE_STATE_COMMON);
    pCommandList->ResourceBarrier(_countof(barriers), barriers);
}

void TransparencyManager::ClearOITResources(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList)
{
    // Clear FragmentList & FragmentListFirstNodeAddress
    CD3DX12_RESOURCE_BARRIER barriers[] = {
        CD3DX12_RESOURCE_BARRIER::Transition(m_pFragmentList, D3D12_RESOURCE_STATE_COMMON,
                                             D3D12_RESOURCE_STATE_COPY_DEST),
        CD3DX12_RESOURCE_BARRIER::Transition(m_pFragmentListFirstNodeAddress, D3D12_RESOURCE_STATE_COMMON,
                                             D3D12_RESOURCE_STATE_UNORDERED_ACCESS)};

    pCommandList->ResourceBarrier(_countof(barriers), barriers);

    DescriptorPool             *pDescriptorPool = m_pRenderer->GetDescriptorPool(threadIndex);
    const UINT                  clearValue[] = {0, 0, 0, 0};
    // Clear OIT FIRST_NODE_ADDRESS (UAV)
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    pDescriptorPool->Alloc(&cpuHandle, &gpuHandle, 1);
    m_pD3DDevice->CopyDescriptorsSimple(1, cpuHandle, m_descriptorTable.cpuHandle,
                                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    ID3D12DescriptorHeap *heaps[] = {pDescriptorPool->GetDescriptorHeap()};
    pCommandList->SetDescriptorHeaps(_countof(heaps), heaps);
    pCommandList->ClearUnorderedAccessViewUint(gpuHandle, m_descriptorTable.cpuHandle, m_pFragmentListFirstNodeAddress,
                                               clearValue, 0, nullptr);

    // Clear FragmentList
    pCommandList->CopyBufferRegion(m_pFragmentList, m_maxFragmentListNodeCount * sizeof(FragmentListNode),
                                   m_pUAVCounterClearResource, 0, sizeof(UINT));

    pCommandList->ResourceBarrier(1,
                                  &CD3DX12_RESOURCE_BARRIER::Transition(m_pFragmentList, D3D12_RESOURCE_STATE_COPY_DEST,
                                                                        D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
}

void TransparencyManager::Cleanup() 
{
    if (m_pUAVCounterClearResource)
    {
        m_pUAVCounterClearResource->Release();
        m_pUAVCounterClearResource = nullptr;
    }

    D3D12ResourceManager *pResourceManager = m_pRenderer->GetResourceManager();
    pResourceManager->DeallocDescriptorTable(&m_descriptorTable);

    for (UINT n = 0; n < MAX_PENDING_FRAME_COUNT; n++)
    {
        if (m_pReadbackBuffers[n])
        {
            m_pReadbackBuffers[n]->Release();
            m_pReadbackBuffers[n] = nullptr;
        }
    }

    CleanupBuffers();

    if (m_pFragmentListRecycleBin)
    {
        delete m_pFragmentListRecycleBin;
        m_pFragmentListRecycleBin = nullptr;
    }
}

void TransparencyManager::OnUpdateWindowSize(UINT width, UINT height) 
{ 
    m_pRenderer->WaitForGPU();

    CleanupBuffers();
    CreateBuffers(width, height);
}

void TransparencyManager::SetRootDescriptorTable(UINT ThreadIndex, ID3D12GraphicsCommandList *pCommandList) 
{
    DescriptorPool* pDescriptorPool = m_pRenderer->GetDescriptorPool(ThreadIndex);
    
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    pDescriptorPool->Alloc(&cpuHandle, &gpuHandle, OIT_DESCRIPTOR_COUNT);

    m_pD3DDevice->CopyDescriptorsSimple(OIT_DESCRIPTOR_COUNT, cpuHandle, m_descriptorTable.cpuHandle,
                                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    pCommandList->SetGraphicsRootDescriptorTable(3, gpuHandle);
}

BOOL TransparencyManager::Initialize(D3D12Renderer *pRenderer, UINT maxFragmentListNodeCount, UINT width, UINT height)
{
    m_pRenderer = pRenderer;
    m_pD3DDevice = pRenderer->GetD3DDevice();

    m_maxFragmentListNodeCount = maxFragmentListNodeCount;
    m_srvDescriptorSize = m_pD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    m_pFragmentListRecycleBin = new D3DResourceRecycleBin;
    m_pFragmentListRecycleBin->Initialize(m_pD3DDevice, D3D12_HEAP_TYPE_DEFAULT,
                                          D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
                                          D3D12_RESOURCE_STATE_COMMON, L"FragmentList");

    CreatDescriptorTable();
    CreateBuffers(width, height);
    CreateUAVCounterClearResource();

    // Create Readback Buffers
    for (UINT n = 0; n < MAX_PENDING_FRAME_COUNT; n++)
    {
        if (FAILED(m_pD3DDevice->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK), D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT)), D3D12_RESOURCE_STATE_COMMON, nullptr,
                IID_PPV_ARGS(&m_pReadbackBuffers[n]))))
        {
            __debugbreak();
        }
    }

    return TRUE;
}

 // UINT  allocatedNodeCount = m_OITFragmentLists[m_curContextIndex].allocatedNodeCount;
// WCHAR debugStr[64] = {L'\0'};
// swprintf_s(debugStr, 64, L"Allocated Node Count: %d\n", allocatedNodeCount);
// OutputDebugStringW(debugStr);

void TransparencyManager::BeginRender(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList)
{
    CopyUAVCounterForRead(pCommandList);

    ClearOITResources(threadIndex, pCommandList);
}

void TransparencyManager::ResolveOIT(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList, D3D12_VIEWPORT *pViewport,
                            D3D12_RECT *pScissorRect, D3D12_CPU_DESCRIPTOR_HANDLE renderTarget)
{
    CD3DX12_RESOURCE_BARRIER barriers[] = {
        CD3DX12_RESOURCE_BARRIER::Transition(m_pFragmentList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                             D3D12_RESOURCE_STATE_COMMON),
        CD3DX12_RESOURCE_BARRIER::Transition(m_pFragmentListFirstNodeAddress, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                             D3D12_RESOURCE_STATE_COMMON)};
    pCommandList->ResourceBarrier(_countof(barriers), barriers);

    DescriptorPool *pDescriptorPool = m_pRenderer->GetDescriptorPool(threadIndex);
    
    // | OIT_DESCRIPTOR_INDEX_FIRST_NODE_ADDRESS_SRV | OIT_DESCRIPTOR_INDEX_FRAGMENT_LIST_NODE_SRV |
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    pDescriptorPool->Alloc(&cpuHandle, &gpuHandle, 2);
    CD3DX12_CPU_DESCRIPTOR_HANDLE srcHandle(m_descriptorTable.cpuHandle, m_srvDescriptorSize, 2);
    m_pD3DDevice->CopyDescriptorsSimple(2, cpuHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    ID3D12DescriptorHeap *pHeaps[] = {pDescriptorPool->GetDescriptorHeap()};
    pCommandList->OMSetRenderTargets(1, &renderTarget, FALSE, nullptr);
    pCommandList->RSSetViewports(1, pViewport);
    pCommandList->RSSetScissorRects(1, pScissorRect);
    pCommandList->SetGraphicsRootSignature(Graphics::OITResolveRS);
    pCommandList->SetPipelineState(Graphics::OITResolvePSO);

    pCommandList->SetDescriptorHeaps(_countof(pHeaps), pHeaps);

    pCommandList->SetGraphicsRootDescriptorTable(0, gpuHandle);
    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCommandList->IASetVertexBuffers(0, 1, &m_pRenderer->GetFullScreenQuadVertexBufferView());
    pCommandList->DrawInstanced(6, 1, 0, 0);
}

void TransparencyManager::EndRender()
{
    ULONGLONG CurTick = GetTickCount64();
    m_pFragmentListRecycleBin->Update(CurTick);

    // prepare next frame
    UINT nextContextIndex = (m_curContextIndex + 1) % MAX_PENDING_FRAME_COUNT;

    UINT *pSysMem = nullptr;
    HRESULT hr = m_pReadbackBuffers[nextContextIndex]->Map(0, nullptr, reinterpret_cast<void **>(&pSysMem));
    if (SUCCEEDED(hr))
    {
        m_allocatedNodeCount = *pSysMem;
        m_pReadbackBuffers[nextContextIndex]->Unmap(0, nullptr);
    }

    m_curContextIndex = nextContextIndex;
}

TransparencyManager::~TransparencyManager() { Cleanup(); }
