#include "pch.h"

#ifdef RENDERER_RAYTRACING
#include "../RendererRaytracing/D3D12Renderer.h"
#include "../RendererRaytracing/GraphicsCommon.h"
#elif defined(RENDERER_D3D12)
#include "../RendererD3D12/D3D12Renderer.h"
#include "../RendererD3D12/GraphicsCommon.h"
#endif

#include "DescriptorPool.h"
#include "D3D12ResourceManager.h"

#include "PrimitiveGenerator.h"

#include "OITManager.h"

void OITManager::CreatDescriptorTables() 
{ 
    D3D12ResourceManager *pResourceManager = m_pRenderer->GetResourceManager();
    
    for (UINT n = 0; n < MAX_PENDING_COUNT; n++)
    {
        pResourceManager->AllocDescriptorTable(&m_OITFragmentLists[n].fragmentListDescriptorTable,
                                               OIT_DESCRIPTOR_COUNT);
    }
}

void OITManager::CreateBuffers(UINT width, UINT height)
{
    for (UINT n = 0; n < MAX_PENDING_COUNT; n++)
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

        // UINT maxFragmentListNodeCount = (UINT)(m_Viewport.Width * m_Viewport.Height);
        UINT maxFragmentListNodeCount = 4096 * 25 * 16;                                             // 16MB
        UINT maxFragmentListbufferSizeInByte = maxFragmentListNodeCount * sizeof(FragmentListNode); // 192MB
        if (FAILED(m_pD3DDevice->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(maxFragmentListbufferSizeInByte + sizeof(UINT),
                                               D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
                D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pFragmentListNode))))
        {
            __debugbreak();
        }
        m_maxFragmentListNodeCount = maxFragmentListNodeCount;

        CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_OITFragmentLists[n].fragmentListDescriptorTable.cpuHandle);

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
        uavDesc.Buffer.NumElements = maxFragmentListNodeCount;
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
        srvDesc.Buffer.NumElements = maxFragmentListNodeCount;
        srvDesc.Buffer.StructureByteStride = sizeof(FragmentListNode);
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        m_pD3DDevice->CreateShaderResourceView(pFragmentListNode, &srvDesc, cpuHandle);
        cpuHandle.Offset(m_srvDescriptorSize);

        m_OITFragmentLists[n].pFragmentList = pFragmentListNode;
        m_OITFragmentLists[n].pFragmentListFirstNodeAddress = pFragmentListNodeFirstAddress;
        m_OITFragmentLists[n].allocatedNodeCount = 0;
    }
}

void OITManager::CleanupBuffers()
{ 
    for (UINT n = 0; n < MAX_PENDING_COUNT; n++)
    {
        if (m_OITFragmentLists[n].pFragmentList)
        {
            m_OITFragmentLists[n].pFragmentList->Release();
            m_OITFragmentLists[n].pFragmentList = nullptr;
        }
        if (m_OITFragmentLists[n].pFragmentListFirstNodeAddress)
        {
            m_OITFragmentLists[n].pFragmentListFirstNodeAddress->Release();
            m_OITFragmentLists[n].pFragmentListFirstNodeAddress = nullptr;
        }
    }
}

void OITManager::Cleanup() 
{
    D3D12ResourceManager *pResourceManager = m_pRenderer->GetResourceManager();
    for (UINT n = 0; n < MAX_PENDING_COUNT; n++)
    {
        pResourceManager->DeallocDescriptorTable(&m_OITFragmentLists[n].fragmentListDescriptorTable);

        if (m_pReadbackBuffers[n])
        {
            m_pReadbackBuffers[n]->Release();
            m_pReadbackBuffers[n] = nullptr;
        }
    }

    CleanupBuffers();
}

void OITManager::OnUpdateWindowSize(UINT width, UINT height) 
{ 
    m_pRenderer->WaitForGPU();

    CleanupBuffers();
    CreateBuffers(width, height);
}

void OITManager::SetRootDescriptorTables(UINT ThreadIndex, ID3D12GraphicsCommandList *pCommandList) 
{
    DescriptorPool* pDescriptorPool = m_pRenderer->GetDescriptorPool(ThreadIndex);
    
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    pDescriptorPool->Alloc(&cpuHandle, &gpuHandle, OIT_DESCRIPTOR_COUNT);

    m_pD3DDevice->CopyDescriptorsSimple(OIT_DESCRIPTOR_COUNT, cpuHandle,
                                        m_OITFragmentLists[m_curContextIndex].fragmentListDescriptorTable.cpuHandle,
                                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    pCommandList->SetGraphicsRootDescriptorTable(3, gpuHandle);
}

BOOL OITManager::Initialize(D3D12Renderer *pRenderer, UINT maxFragmentListNodeCount, UINT width, UINT height)
{
    m_pRenderer = pRenderer;
    m_pD3DDevice = pRenderer->GetD3DDevice();

    m_srvDescriptorSize = m_pD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    CreatDescriptorTables();
    CreateBuffers(width, height);

    // Create Readback Buffers
    for (UINT n = 0; n < MAX_PENDING_COUNT; n++)
    {
        if (FAILED(m_pD3DDevice->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK), D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT)), D3D12_RESOURCE_STATE_COMMON, nullptr,
                IID_PPV_ARGS(&m_pReadbackBuffers[n]))))
        {
            __debugbreak();
        }
    }
}

void OITManager::BeginRender(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList)
{
    DescriptorPool *pDescriptorPool = m_pRenderer->GetDescriptorPool(threadIndex);

    // | OIT_DESCRIPTOR_INDEX_FIRST_NODE_ADDRESS_UAV |
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    pDescriptorPool->Alloc(&cpuHandle, &gpuHandle, 1);
    m_pD3DDevice->CopyDescriptorsSimple(1, cpuHandle,
                                        m_OITFragmentLists[m_curContextIndex].fragmentListDescriptorTable.cpuHandle,
                                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    const UINT clearValue[] = {0, 0, 0, 0};

    ID3D12Resource *pFragmentList = m_OITFragmentLists[m_curContextIndex].pFragmentList;
    ID3D12Resource *pFragmentListFirstNodeAddress = m_OITFragmentLists[m_curContextIndex].pFragmentListFirstNodeAddress;
    ID3D12Resource *pReadbackBuffer = m_pReadbackBuffers[m_curContextIndex];

    //UINT  allocatedNodeCount = m_OITFragmentLists[m_curContextIndex].allocatedNodeCount;
    //WCHAR debugStr[64] = {L'\0'};
    //swprintf_s(debugStr, 64, L"Allocated Node Count: %d\n", allocatedNodeCount);
    //OutputDebugStringW(debugStr);

    CD3DX12_RESOURCE_BARRIER readbackBarriers[] = {
        CD3DX12_RESOURCE_BARRIER::Transition(pFragmentList, D3D12_RESOURCE_STATE_COMMON,
                                             D3D12_RESOURCE_STATE_COPY_SOURCE),
        CD3DX12_RESOURCE_BARRIER::Transition(pReadbackBuffer, D3D12_RESOURCE_STATE_COMMON,
                                             D3D12_RESOURCE_STATE_COPY_DEST)};
    pCommandList->ResourceBarrier(_countof(readbackBarriers), readbackBarriers);

    pCommandList->CopyBufferRegion(pReadbackBuffer, 0, pFragmentList,
                                   m_maxFragmentListNodeCount * sizeof(FragmentListNode), sizeof(UINT));

    CD3DX12_RESOURCE_BARRIER barriers[] = {
        CD3DX12_RESOURCE_BARRIER::Transition(pFragmentList, D3D12_RESOURCE_STATE_COPY_SOURCE,
                                             D3D12_RESOURCE_STATE_COPY_DEST),
        CD3DX12_RESOURCE_BARRIER::Transition(pFragmentListFirstNodeAddress, D3D12_RESOURCE_STATE_COMMON,
                                             D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
        CD3DX12_RESOURCE_BARRIER::Transition(pReadbackBuffer, D3D12_RESOURCE_STATE_COPY_DEST,
                                             D3D12_RESOURCE_STATE_COMMON)};
    pCommandList->ResourceBarrier(_countof(barriers), barriers);

    ID3D12DescriptorHeap *heaps[] = {pDescriptorPool->GetDescriptorHeap()};
    pCommandList->SetDescriptorHeaps(_countof(heaps), heaps);
    pCommandList->ClearUnorderedAccessViewUint(gpuHandle, cpuHandle, pFragmentListFirstNodeAddress, clearValue, 0,
                                               nullptr);

    pCommandList->CopyBufferRegion(pFragmentList, m_maxFragmentListNodeCount * sizeof(FragmentListNode),
                                   m_pUAVCounterClearResource, 0, sizeof(UINT));

    pCommandList->ResourceBarrier(1,
                                  &CD3DX12_RESOURCE_BARRIER::Transition(pFragmentList, D3D12_RESOURCE_STATE_COPY_DEST,
                                                                        D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
}

void OITManager::ResolveOIT(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList, D3D12_VIEWPORT *pViewport,
                            D3D12_RECT *pScissorRect, D3D12_CPU_DESCRIPTOR_HANDLE renderTarget)
{
    DescriptorPool *pDescriptorPool = m_pRenderer->GetDescriptorPool(threadIndex);
    
    // | OIT_DESCRIPTOR_INDEX_FIRST_NODE_ADDRESS_SRV | OIT_DESCRIPTOR_INDEX_FRAGMENT_LIST_NODE_SRV |
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    pDescriptorPool->Alloc(&cpuHandle, &gpuHandle, 2);

    CD3DX12_CPU_DESCRIPTOR_HANDLE srcHandle(m_OITFragmentLists[m_curContextIndex].fragmentListDescriptorTable.cpuHandle,
                                            m_srvDescriptorSize, 2);

    m_pD3DDevice->CopyDescriptorsSimple(OIT_DESCRIPTOR_COUNT, cpuHandle, srcHandle,
                                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    ID3D12DescriptorHeap *pHeaps[] = {pDescriptorPool->GetDescriptorHeap()};

    pCommandList->OMSetRenderTargets(1, &renderTarget, FALSE, nullptr);
    pCommandList->RSSetViewports(1, pViewport);
    pCommandList->RSSetScissorRects(1, pScissorRect);
    pCommandList->SetGraphicsRootSignature(Graphics::OITResolveRS);
    pCommandList->SetPipelineState(Graphics::OITResolvePSO);

    pCommandList->SetDescriptorHeaps(_countof(pHeaps), pHeaps);

    pCommandList->SetGraphicsRootDescriptorTable(0, gpuHandle);
    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    const BASIC_MESH &quadMesh = PrimitiveGenerator::GetQuadMesh();
    pCommandList->IASetVertexBuffers(0, 1, &quadMesh.VertexBufferView);
    pCommandList->IASetIndexBuffer(&quadMesh.IndexBufferView);
    pCommandList->DrawIndexedInstanced(quadMesh.numIndices, 1, 0, 0, 0);
}

void OITManager::EndRender()
{
    // prepare next frame
    UINT nextContextIndex = (m_curContextIndex + 1) % MAX_PENDING_COUNT;

    UINT *pSysMem = nullptr;
    HRESULT hr = m_pReadbackBuffers[nextContextIndex]->Map(0, nullptr, reinterpret_cast<void **>(&pSysMem));
    if (SUCCEEDED(hr))
    {
        m_OITFragmentLists[nextContextIndex].allocatedNodeCount = *pSysMem;
        m_pReadbackBuffers[nextContextIndex]->Unmap(0, nullptr);
    }

    m_curContextIndex = nextContextIndex;
}