#include "pch.h"

#include "ConstantBufferPool.h"
#include "D3D12Renderer.h"
#include "D3D12ResourceManager.h"
#include "DescriptorPool.h"
#include "GraphicsCommon.h"
#include "RaytracingMeshObject.h"
#include "ShaderTable.h"
#include "D3DResourceRecycleBin.h"

#include "RaytracingManager.h"

void RaytracingManager::Cleanup()
{
    if (m_pResourceBinBLAS)
    {
        delete m_pResourceBinBLAS;
        m_pResourceBinBLAS = nullptr;
    }
    if (m_pResourceBinTLAS)
    {
        delete m_pResourceBinTLAS;
        m_pResourceBinTLAS = nullptr;
    }
    if (m_pResourceBinScratchResource)
    {
        delete m_pResourceBinScratchResource;
        m_pResourceBinScratchResource = nullptr;
    }
    if (m_pResourceBinTLASInstanceDescList)
    {
        delete m_pResourceBinTLASInstanceDescList;
        m_pResourceBinTLASInstanceDescList = nullptr;
    }

    m_pRenderer->GetResourceManager()->DeallocDescriptorTable(&m_TLASHandle);
    if (m_topLevelASBuffers.pInstanceDesc)
    {
        if (m_pInstanceDescsCPU)
        {
            m_topLevelASBuffers.pInstanceDesc->Unmap(0, nullptr);
            m_pInstanceDescsCPU = nullptr;
        }
        m_topLevelASBuffers.pInstanceDesc->Release();
        m_topLevelASBuffers.pInstanceDesc = nullptr;
    }
    if (m_topLevelASBuffers.pResult)
    {
        m_topLevelASBuffers.pResult->Release();
        m_topLevelASBuffers.pResult = nullptr;
    }
    if (m_topLevelASBuffers.pScratch)
    {
        m_topLevelASBuffers.pScratch->Release();
        m_topLevelASBuffers.pScratch = nullptr;
    }
    if (m_pRayGenShaderTable)
    {
        delete m_pRayGenShaderTable;
        m_pRayGenShaderTable = nullptr;
    }
    if (m_pMissShaderTable)
    {
        delete m_pMissShaderTable;
        m_pMissShaderTable = nullptr;
    }
    for (UINT i = 0; i < MAX_PENDING_FRAME_COUNT; i++)
    {
        if (m_pHitShaderTables[i])
        {
            delete m_pHitShaderTables[i];
            m_pHitShaderTables[i] = nullptr;
        }
    }
}

BOOL RaytracingManager::Initialize(D3D12Renderer *pRnd, ID3D12GraphicsCommandList4 *pCommandList, UINT maxInstanceCount,
                                   UINT maxThreadCount)
{
    BOOL                                               result = FALSE;
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
    ID3D12Resource                                    *pInstanceDescs = nullptr;
    ID3D12Resource                                    *pResult = nullptr;
    ID3D12Resource                                    *pScratch = nullptr;

    m_pRenderer = pRnd;

    ID3D12Device5 *pD3DDevice = pRnd->GetD3DDevice();
    
    m_pResourceBinBLAS = new D3DResourceRecycleBin;
    m_pResourceBinBLAS->Initialize(pD3DDevice, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
                                   D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
                                   L"BottomLevelAccelerationStructure");
    m_pResourceBinTLAS = new D3DResourceRecycleBin;
    m_pResourceBinTLAS->Initialize(pD3DDevice, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
                                   D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
                                   L"TopLevelAccelerationStructure");
    m_pResourceBinScratchResource = new D3DResourceRecycleBin;
    m_pResourceBinScratchResource->Initialize(pD3DDevice, D3D12_HEAP_TYPE_DEFAULT,
                                              D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON,
                                              L"ScratchResource");
    m_pResourceBinTLASInstanceDescList = new D3DResourceRecycleBin;
    m_pResourceBinTLASInstanceDescList->Initialize(pD3DDevice, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_FLAG_NONE,
                                               D3D12_RESOURCE_STATE_GENERIC_READ, L"InstanceDesces");
    InitializeCriticalSection(&m_cs);
    InitializeConditionVariable(&m_cv);

    m_maxInstanceCount = maxInstanceCount;

    // TODO: TopLevelAS Initialize
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {};
    topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    topLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    topLevelInputs.NumDescs = maxInstanceCount;
    topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    m_topLevelInputs = topLevelInputs;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};

    pD3DDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
    if (topLevelPrebuildInfo.ResultDataMaxSizeInBytes < 0)
    {
        __debugbreak();
        goto lb_return;
    }

    // Create Scratch Buffer
    if (FAILED(pD3DDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(topLevelPrebuildInfo.ScratchDataSizeInBytes,
                                           D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
            D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pScratch))))
    {
        __debugbreak();
        goto lb_return;
    }
    // Create Result Buffer;
    if (FAILED(pD3DDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(topLevelPrebuildInfo.ResultDataMaxSizeInBytes,
                                           D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
            D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, nullptr, IID_PPV_ARGS(&pResult))))
    {
        __debugbreak();
        goto lb_return;
    }
    // Create InstanceDescs Buffer
    UINT instanceDescSizeInBytes = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * maxInstanceCount;
    if (FAILED(pD3DDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(instanceDescSizeInBytes), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&pInstanceDescs))))
    {
        __debugbreak();
        goto lb_return;
    }

    // Map을 하고 안닫을게!
    pInstanceDescs->Map(0, nullptr, reinterpret_cast<void **>(&m_pInstanceDescsCPU));
    ZeroMemory(m_pInstanceDescsCPU, instanceDescSizeInBytes);

    topLevelInputs.InstanceDescs = pInstanceDescs->GetGPUVirtualAddress();
    topLevelBuildDesc.Inputs = topLevelInputs;
    topLevelBuildDesc.DestAccelerationStructureData = pResult->GetGPUVirtualAddress();
    topLevelBuildDesc.ScratchAccelerationStructureData = pScratch->GetGPUVirtualAddress();

    pCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);

    m_topLevelASBuffers.pInstanceDesc = pInstanceDescs;
    m_topLevelASBuffers.pResult = pResult;
    m_topLevelASBuffers.pScratch = pScratch;

    // Create SRV
    m_pRenderer->GetResourceManager()->AllocDescriptorTable(&m_TLASHandle, 1);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.RaytracingAccelerationStructure.Location = m_topLevelASBuffers.pResult->GetGPUVirtualAddress();
    pD3DDevice->CreateShaderResourceView(nullptr, &srvDesc, m_TLASHandle.cpuHandle);

    CreateShaderTables();

    m_maxThreadCount = maxThreadCount;

    result = TRUE;
lb_return:
    return result;
}

void RaytracingManager::CreateTopLevelAS(ID3D12GraphicsCommandList4 *pCommandList)
{
    m_topLevelInputs.InstanceDescs = m_topLevelASBuffers.pInstanceDesc->GetGPUVirtualAddress();
    m_topLevelInputs.NumDescs = m_AllocatedInstanceCount;

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
    buildDesc.Inputs = m_topLevelInputs;
    buildDesc.DestAccelerationStructureData = m_topLevelASBuffers.pResult->GetGPUVirtualAddress();
    buildDesc.ScratchAccelerationStructureData = m_topLevelASBuffers.pScratch->GetGPUVirtualAddress();

    // Build the top-level AS
    pCommandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

    // Wait for the builder to complete by setting a barrier on the resulting
    // buffer. This can be important in case the rendering is triggered
    // immediately afterwards, without executing the command list
    D3D12_RESOURCE_BARRIER uavBarrier;
    uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    uavBarrier.UAV.pResource = m_topLevelASBuffers.pResult;
    uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    pCommandList->ResourceBarrier(1, &uavBarrier);
}

void RaytracingManager::CreateShaderTables()
{
    ID3D12Device5 *pD3DDevice = m_pRenderer->GetD3DDevice();

    void *rayGenShaderIdentifier = nullptr;
    void *missShaderIdentifier = nullptr;
    void *shadowMissShaderIdentifier = nullptr;
    void *hitGroupShaderIdentifier = nullptr;
    void *shadowHitGroupShaderIdentifier = nullptr;

    auto *stateObjProps = Graphics::rtStateObjectProps;
#ifdef USE_DEFERRED_RENDERING
    rayGenShaderIdentifier = stateObjProps->GetShaderIdentifier(L"DeferredRayGen");
#else
    rayGenShaderIdentifier = stateObjProps->GetShaderIdentifier(L"RayGen");
#endif // USE_DEFERRED_RENDERING

    missShaderIdentifier = stateObjProps->GetShaderIdentifier(L"Miss");
    shadowMissShaderIdentifier = stateObjProps->GetShaderIdentifier(L"ShadowMiss");
    hitGroupShaderIdentifier = stateObjProps->GetShaderIdentifier(L"HitGroup");
    shadowHitGroupShaderIdentifier = stateObjProps->GetShaderIdentifier(L"ShadowHitGroup");
    UINT shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

    // RayGen Shader Table
    {
        UINT shaderRecordSize = shaderIdentifierSize;

        m_pRayGenShaderTable = new ShaderTable;
        m_pRayGenShaderTable->Initialize(pD3DDevice, 1, shaderRecordSize, L"RayGenShaderTable");
        m_pRayGenShaderTable->InsertRecord(rayGenShaderIdentifier, shaderIdentifierSize);
    }
    // Miss Shader Table
    {
        UINT shaderRecordSize = shaderIdentifierSize;

        m_pMissShaderTable = new ShaderTable;
        m_pMissShaderTable->Initialize(pD3DDevice, 2, shaderRecordSize, L"MissShaderTable");
        m_pMissShaderTable->InsertRecord(missShaderIdentifier, shaderIdentifierSize);
        m_pMissShaderTable->InsertRecord(shadowMissShaderIdentifier, shaderIdentifierSize);
    }
    // Hit Shader Table
    for (UINT i = 0; i < MAX_PENDING_FRAME_COUNT; i++)
    {
        UINT shaderRecordSize = shaderIdentifierSize + sizeof(Graphics::LOCAL_ROOT_ARG);

        m_pHitShaderTables[i] = new ShaderTable;
        m_pHitShaderTables[i]->Initialize(
            pD3DDevice, m_maxInstanceCount * RaytracingMeshObject::MAX_DESCRIPTOR_COUNT_PER_BLAS, shaderRecordSize,
            L"HitGroupShaderTable"); // TODO: Shader Record 개수를 gemetry 개수를 고려해서 변경하기
    }
}

void RaytracingManager::InsertBLASInstance(ID3D12Resource *pBLAS, const Matrix *pTM, UINT instanceID,
                                           Graphics::LOCAL_ROOT_ARG *pRootArgs, UINT numFaceGroup)
{
    if (m_AllocatedInstanceCount >= m_maxInstanceCount)
        __debugbreak();

    UINT instanceId = m_AllocatedInstanceCount;
    m_AllocatedInstanceCount++;

    // Insert Hit Group Record
    void *hitGroupShaderIdentifier = Graphics::rtStateObjectProps->GetShaderIdentifier(L"HitGroup");
    void *shadowHitGroupShaderIdentifier = Graphics::rtStateObjectProps->GetShaderIdentifier(L"ShadowHitGroup");
    UINT  shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    UINT  shaderRecordOffset = m_pHitShaderTables[m_curContextIndex]->GetAllocatedRecordCount();

    for (UINT i = 0; i < numFaceGroup; i++)
    {
        Graphics::LOCAL_ROOT_ARG *pRootArgPerGeometry = pRootArgs + i;

        m_pHitShaderTables[m_curContextIndex]->InsertRecord(hitGroupShaderIdentifier, shaderIdentifierSize,
                                                            pRootArgPerGeometry, sizeof(Graphics::LOCAL_ROOT_ARG));
        m_pHitShaderTables[m_curContextIndex]->InsertRecord(shadowHitGroupShaderIdentifier, shaderIdentifierSize,
                                                            pRootArgPerGeometry, sizeof(Graphics::LOCAL_ROOT_ARG));
    }
    
    D3D12_RAYTRACING_INSTANCE_DESC *pInstanceDesc = m_pInstanceDescsCPU + instanceId;
    memcpy(pInstanceDesc->Transform, &pTM->Transpose(), sizeof(pInstanceDesc->Transform));

    pInstanceDesc->AccelerationStructure = pBLAS->GetGPUVirtualAddress();
    pInstanceDesc->InstanceID = instanceId;
    pInstanceDesc->InstanceContributionToHitGroupIndex = shaderRecordOffset;
    pInstanceDesc->Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
    pInstanceDesc->InstanceMask = ~0;
    //WakeConditionVariable(&m_cv);
}

void RaytracingManager::InsertBLASInstanceAsync(ID3D12Resource *pBLAS, const Matrix *pTM, UINT instanceID,
                                                Graphics::LOCAL_ROOT_ARG *pRootArgs, UINT numFaceGroup)
{
    EnterCriticalSection(&m_cs);
    InsertBLASInstance(pBLAS, pTM, instanceID, pRootArgs, numFaceGroup);
    LeaveCriticalSection(&m_cs);
}

void RaytracingManager::Reset()
{
    m_AllocatedInstanceCount = 0;
    m_pHitShaderTables[m_curContextIndex]->Reset();

    m_curContextIndex = (m_curContextIndex + 1) % MAX_PENDING_FRAME_COUNT;
    /*m_pMissShaderTable->Reset();
    m_pRayGenShaderTable->Reset();*/
    //UpdateManagedResource();
}

void RaytracingManager::UpdateManagedResource()
{
    ULONGLONG CurTick = GetTickCount64();
    m_pResourceBinTLAS->Update(CurTick);
    m_pResourceBinBLAS->Update(CurTick);
    m_pResourceBinScratchResource->Update(CurTick);
    m_pResourceBinTLASInstanceDescList->Update(CurTick);
}

RaytracingManager::~RaytracingManager() { Cleanup(); }

void RaytracingManager::DispatchRay(UINT threadIndex, ID3D12GraphicsCommandList4 *pCommandList,
                                    ID3D12Resource *pOutputView,
                                    D3D12_CPU_DESCRIPTOR_HANDLE outputViewUav)
{
    ID3D12Device5        *pD3DDevice = m_pRenderer->GetD3DDevice();
    DescriptorPool       *pDescriptorPool = m_pRenderer->GetDescriptorPool(threadIndex);
    ID3D12DescriptorHeap *pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();

    UINT descriptorSize = pD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandles;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandles;

    pDescriptorPool->Alloc(&cpuHandles, &gpuHandles, 2); // UAV, SRV

    pD3DDevice->CopyDescriptorsSimple(
        1, CD3DX12_CPU_DESCRIPTOR_HANDLE(cpuHandles, Graphics::GLOBAL_ROOT_PARAM_INDEX_OUTPUTVIEW, descriptorSize),
        outputViewUav, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    pD3DDevice->CopyDescriptorsSimple(
        1,
        CD3DX12_CPU_DESCRIPTOR_HANDLE(cpuHandles, Graphics::GLOBAL_ROOT_PARAM_INDEX_ACCELERATIONSTRUCTURE,
                                      descriptorSize),
        m_TLASHandle.cpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    pCommandList->ResourceBarrier(1,
                                  &CD3DX12_RESOURCE_BARRIER::Transition(pOutputView, D3D12_RESOURCE_STATE_COPY_SOURCE,
                                                                        D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

    pCommandList->SetComputeRootSignature(Graphics::globalRS);
    pCommandList->SetDescriptorHeaps(1, &pDescriptorHeap);
    pCommandList->SetComputeRootDescriptorTable(0, gpuHandles);
    pCommandList->SetComputeRootDescriptorTable(1, m_pRenderer->GetGlobalDescriptorHandle(threadIndex));

    ID3D12Resource *pHitShaderTable = m_pHitShaderTables[m_curContextIndex]->GetResource();
    ID3D12Resource *pMissShaderTable = m_pMissShaderTable->GetResource();
    ID3D12Resource *pRayGenShaderTable = m_pRayGenShaderTable->GetResource();

    D3D12_DISPATCH_RAYS_DESC desc = {};
    desc.HitGroupTable.StartAddress = pHitShaderTable->GetGPUVirtualAddress();
    desc.HitGroupTable.SizeInBytes = m_pHitShaderTables[m_curContextIndex]->GetSizeInBytes();
    desc.HitGroupTable.StrideInBytes = m_pHitShaderTables[m_curContextIndex]->GetShaderRecordSize();
    desc.MissShaderTable.StartAddress = pMissShaderTable->GetGPUVirtualAddress();
    desc.MissShaderTable.SizeInBytes = m_pMissShaderTable->GetSizeInBytes();
    desc.MissShaderTable.StrideInBytes = m_pMissShaderTable->GetShaderRecordSize();
    desc.RayGenerationShaderRecord.StartAddress = pRayGenShaderTable->GetGPUVirtualAddress();
    desc.RayGenerationShaderRecord.SizeInBytes = pRayGenShaderTable->GetDesc().Width;
    desc.Width = pOutputView->GetDesc().Width;
    desc.Height = pOutputView->GetDesc().Height;
    desc.Depth = 3;

    pCommandList->SetPipelineState1(Graphics::rtStateObject);
    pCommandList->DispatchRays(&desc);

    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pOutputView,
                                                                           D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                                                           D3D12_RESOURCE_STATE_COPY_SOURCE));
}

void RaytracingManager::DispatchRay(UINT threadIndex, ID3D12GraphicsCommandList4 *pCommandList,
                                    ID3D12Resource *pOutputView,
                                    D3D12_CPU_DESCRIPTOR_HANDLE outputViewUav, D3D12_CPU_DESCRIPTOR_HANDLE gbuffers,
                                    UINT gbufferCount)
{
    ID3D12Device5        *pD3DDevice = m_pRenderer->GetD3DDevice();
    DescriptorPool       *pDescriptorPool = m_pRenderer->GetDescriptorPool(threadIndex);
    ID3D12DescriptorHeap *pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();

    UINT descriptorSize = m_pRenderer->GetSRVDescriptorSize();

    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandles;
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandles;
    // | OutputView | TLAS | g-buffers |
    pDescriptorPool->Alloc(&cpuHandles, &gpuHandles, 2 + gbufferCount); 

    pD3DDevice->CopyDescriptorsSimple(1, cpuHandles, outputViewUav, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    cpuHandles.Offset(descriptorSize);
    pD3DDevice->CopyDescriptorsSimple(1, cpuHandles, m_TLASHandle.cpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    cpuHandles.Offset(descriptorSize);
    pD3DDevice->CopyDescriptorsSimple(gbufferCount, cpuHandles, gbuffers, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    pCommandList->ResourceBarrier(1,
                                  &CD3DX12_RESOURCE_BARRIER::Transition(pOutputView, D3D12_RESOURCE_STATE_COMMON,
                                                                        D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

    pCommandList->SetComputeRootSignature(Graphics::globalRS);
    pCommandList->SetDescriptorHeaps(1, &pDescriptorHeap);
    pCommandList->SetComputeRootDescriptorTable(0, gpuHandles);
    pCommandList->SetComputeRootDescriptorTable(1, m_pRenderer->GetGlobalDescriptorHandle(threadIndex));

    ID3D12Resource *pHitShaderTable = m_pHitShaderTables[m_curContextIndex]->GetResource();
    ID3D12Resource *pMissShaderTable = m_pMissShaderTable->GetResource();
    ID3D12Resource *pRayGenShaderTable = m_pRayGenShaderTable->GetResource();

    D3D12_DISPATCH_RAYS_DESC desc = {};
    desc.HitGroupTable.StartAddress = pHitShaderTable->GetGPUVirtualAddress();
    desc.HitGroupTable.SizeInBytes = m_pHitShaderTables[m_curContextIndex]->GetSizeInBytes();
    desc.HitGroupTable.StrideInBytes = m_pHitShaderTables[m_curContextIndex]->GetShaderRecordSize();
    desc.MissShaderTable.StartAddress = pMissShaderTable->GetGPUVirtualAddress();
    desc.MissShaderTable.SizeInBytes = m_pMissShaderTable->GetSizeInBytes();
    desc.MissShaderTable.StrideInBytes = m_pMissShaderTable->GetShaderRecordSize();
    desc.RayGenerationShaderRecord.StartAddress = pRayGenShaderTable->GetGPUVirtualAddress();
    desc.RayGenerationShaderRecord.SizeInBytes = pRayGenShaderTable->GetDesc().Width;
    desc.Width = pOutputView->GetDesc().Width;
    desc.Height = pOutputView->GetDesc().Height;
    desc.Depth = 3;

    pCommandList->SetPipelineState1(Graphics::rtStateObject);
    pCommandList->DispatchRays(&desc);

    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pOutputView,
                                                                           D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                                                           D3D12_RESOURCE_STATE_COMMON));
}
