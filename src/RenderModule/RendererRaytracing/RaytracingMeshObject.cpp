#include "pch.h"

#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <d3dx12.h>

#include "ConstantBuffers.h"

#include "ConstantBufferPool.h"
#include "D3D12Renderer.h"
#include "D3D12ResourceManager.h"
#include "DescriptorPool.h"

#include "MaterialManager.h"

#include "ShaderTable.h"

#include "RaytracingManager.h"

#include "RaytracingMeshObject.h"

// 레이트레이싱 SRV 디스크립터 구조
// | VertexBuffer(SRV) | IndexBuffer0(SRV) | DiffuseTex0(SRV) | ... |
BOOL RaytracingMeshObject::Initialize(D3D12Renderer *pRenderer, RENDER_ITEM_TYPE type)
{
    ID3D12Device5 *pDevice = pRenderer->GetD3DDevice();
    m_descriptorSize = pRenderer->GetSRVDescriptorSize();

    m_pRenderer = pRenderer;
    m_pD3DDevice = pDevice;
    m_type = type;

    return TRUE;
}

void RaytracingMeshObject::UpdateBoneMatrices(Keyframe **ppKeyframes, UINT frameCount)
{
    for (uint32_t boneId = 0; boneId < m_jointCount; boneId++)
    {
        Joint    *pJoint = m_pJoints + boneId;
        Keyframe *pKeyframe = ppKeyframes[boneId];

        const int    parentIdx = pJoint->parentIndex;
        const Matrix parentMatrix = parentIdx >= 0 ? m_pBoneMatrices[parentIdx] : Matrix::Identity;

        int    numKeys = pKeyframe->NumKeys;
        Matrix TM = numKeys > 0 ? pKeyframe->pKeys[frameCount % numKeys] : Matrix::Identity;

        m_pBoneMatrices[boneId] = TM * parentMatrix;
    }
    for (uint32_t boneId = 0; boneId < m_jointCount; boneId++)
    {
        m_pBoneMatrices[boneId] = (m_pJoints[boneId].globalBindposeInverse * m_pBoneMatrices[boneId]).Transpose();
    }
}

/*
 *  Descriptor Table Per Obj - Offset : 0
 *  Basic Mesh :   | World TM |
 *  Skinned Mesh:  | World TM | Bone Matrices |
 */
void RaytracingMeshObject::UpdateDescriptorTablePerObj(D3D12_CPU_DESCRIPTOR_HANDLE descriptorTable, UINT threadIndex,
                                                       const Matrix *pWorldMat, UINT numInstance,
                                                       Keyframe **ppKeyframes, UINT frameCount)
{
    CB_CONTAINER       *pSkinnedCB;
    CB_CONTAINER       *pMeshCB;
    ConstantBufferPool *pMeshConstantBufferPool;
    ConstantBufferPool *pSkinnedConstantBufferPool;

    pMeshConstantBufferPool = m_pRenderer->GetConstantBufferPool(CONSTANT_BUFFER_TYPE_MESH, threadIndex);

    // Update World Transform Constant Buffer
    pMeshCB = pMeshConstantBufferPool->Alloc(numInstance);

    MeshConstants *pMeshConsts = (MeshConstants *)pMeshCB->pSystemMemAddr;
    for (UINT i = 0; i < numInstance; i++)
    {
        MeshConstants *pMeshConst = pMeshConsts + i;
        pMeshConst->world = pWorldMat[i].Transpose();
        pMeshConst->worldIT = pWorldMat[i].Invert().Transpose();
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE dest = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorTable);
    m_pD3DDevice->CopyDescriptorsSimple(DESCRIPTOR_COUNT_PER_STATIC_OBJ, dest, pMeshCB->CBVHandle,
                                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    if (m_type == RENDER_ITEM_TYPE_CHAR_OBJ && ppKeyframes != nullptr)
    {
        dest.Offset(m_descriptorSize);
        pSkinnedConstantBufferPool = m_pRenderer->GetConstantBufferPool(CONSTANT_BUFFER_TYPE_SKINNED, threadIndex);
        pSkinnedCB = pSkinnedConstantBufferPool->Alloc();
        memcpy(pSkinnedCB->pSystemMemAddr, m_pBoneMatrices, sizeof(Matrix) * m_jointCount);

        m_pD3DDevice->CopyDescriptorsSimple(1, dest, pSkinnedCB->CBVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }
}

/*
 *  Descriptor Table Per FaceGroup - Offset : 0
 *  | Material | Textures |
 */
void RaytracingMeshObject::UpdateDescriptorTablePerFaceGroup(D3D12_CPU_DESCRIPTOR_HANDLE descriptorTable,
                                                             UINT threadIndex, IRenderMaterial *const *ppMaterials,
                                                             UINT numMaterial)
{
    DASSERT(numMaterial == m_faceGroupCount);

    CB_CONTAINER       *pGeomCBs;
    ConstantBufferPool *pGeometryConstantBufferPool =
        m_pRenderer->GetConstantBufferPool(CONSTANT_BUFFER_TYPE_GEOMETRY, threadIndex);

    CD3DX12_CPU_DESCRIPTOR_HANDLE dest(descriptorTable);
    pGeomCBs = pGeometryConstantBufferPool->Alloc(m_faceGroupCount);
    for (UINT i = 0; i < m_faceGroupCount; i++)
    {
        INDEXED_FACE_GROUP *pFaceGroup = m_pFaceGroups + i;
        CB_CONTAINER       *pGeomCB = pGeomCBs + i;
        FaceGroupConstants *pGeometry = (FaceGroupConstants *)pGeomCB->pSystemMemAddr;

        MATERIAL_HANDLE *pMatHandle = (MATERIAL_HANDLE *)ppMaterials[i];

        pGeometry->materialIndex = pMatHandle->index;
        pGeometry->useHeightMap = !pMatHandle->pHeightTexHandle ? FALSE : TRUE;
        pGeometry->useMaterial = !pMatHandle->pAlbedoTexHandle ? FALSE : TRUE;

        m_pD3DDevice->CopyDescriptorsSimple(1, dest, pGeomCB->CBVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        dest.Offset(m_descriptorSize);

        pMatHandle->CopyDescriptors(m_pD3DDevice, dest, m_descriptorSize);
        dest.Offset(m_descriptorSize, MATERIAL_HANDLE::DESCRIPTOR_SIZE);
    }
}

void RaytracingMeshObject::Draw(UINT threadIndex, ID3D12GraphicsCommandList4 *pCommandList, const Matrix *pWorldMat,
                                DRAW_PASS_TYPE passType, FILL_MODE fillMode, Keyframe **ppKeyframes, UINT frameCount)
{
    DrawWithMaterial(threadIndex, pCommandList, pWorldMat, m_ppMaterials, m_faceGroupCount, passType, fillMode,
                     ppKeyframes, frameCount);
}

void RaytracingMeshObject::DrawWithMaterial(UINT threadIndex, ID3D12GraphicsCommandList4 *pCommandList,
                                            const Matrix *pWorldMat, IRenderMaterial *const *ppMaterials,
                                            UINT numMaterials, DRAW_PASS_TYPE passType, FILL_MODE fillMode,
                                            Keyframe **ppKeyframes, UINT frameCount)
{
#ifdef USE_DEFERRED_RENDERING
    DescriptorPool       *pDescriptorPool = m_pRenderer->GetDescriptorPool(threadIndex);
    ID3D12DescriptorHeap *pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandlePerObj = {};
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandlePerObj = {};
    if (m_type == RENDER_ITEM_TYPE_MESH_OBJ)
    {
        pDescriptorPool->Alloc(&cpuHandlePerObj, &gpuHandlePerObj, DESCRIPTOR_COUNT_PER_BASIC_OBJECT);
    }
    else if (m_type == RENDER_ITEM_TYPE_CHAR_OBJ && ppKeyframes != nullptr)
    {
        UpdateBoneMatrices(ppKeyframes, frameCount);
        pDescriptorPool->Alloc(&cpuHandlePerObj, &gpuHandlePerObj, DESCRIPTOR_COUNT_PER_SKINNED_OBJECT);
    }
    UpdateDescriptorTablePerObj(cpuHandlePerObj, threadIndex, pWorldMat, 1, ppKeyframes, frameCount);

    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandlePerFaceGroup = {};
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandlePerFaceGroup = {};
    pDescriptorPool->Alloc(&cpuHandlePerFaceGroup, &gpuHandlePerFaceGroup,
                           DESCRIPTOR_COUNT_PER_MATERIAL * m_faceGroupCount);
    UpdateDescriptorTablePerFaceGroup(cpuHandlePerFaceGroup, threadIndex, ppMaterials, numMaterials);

    // set RootSignature
    pCommandList->SetGraphicsRootSignature(Graphics::GetRS(m_type, passType));
    ID3D12DescriptorHeap *ppHeaps[] = {pDescriptorHeap};
    pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    pCommandList->SetPipelineState(Graphics::GetPSO(m_type, passType, fillMode));
    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    pCommandList->SetGraphicsRootDescriptorTable(0, m_pRenderer->GetGlobalDescriptorHandle(threadIndex));
    pCommandList->SetGraphicsRootDescriptorTable(1, gpuHandlePerObj);

    if (passType == DRAW_PASS_TYPE_TRANSPARENCY)
    {
        pCommandList->SetGraphicsRootDescriptorTable(3, m_pRenderer->GetOITDescriptorHandle(threadIndex));
    }

    for (UINT i = 0; i < m_faceGroupCount; i++)
    {
        MATERIAL_TYPE mType = ppMaterials[i]->GetType();
        if ((mType == MATERIAL_TYPE_TRANSLUCENT && passType == DRAW_PASS_TYPE_TRANSPARENCY) ||
            (mType == MATERIAL_TYPE_DEFAULT && passType != DRAW_PASS_TYPE_TRANSPARENCY))
        {
            pCommandList->SetGraphicsRootDescriptorTable(2, gpuHandlePerFaceGroup);
            gpuHandlePerFaceGroup.Offset(m_descriptorSize, DESCRIPTOR_INDEX_PER_FACE_GROUP_COUNT);

            INDEXED_FACE_GROUP *pFaceGroup = m_pFaceGroups + i;
            pCommandList->IASetIndexBuffer(&pFaceGroup->IndexBufferView);
            pCommandList->DrawIndexedInstanced(pFaceGroup->numTriangles * 3, 1, 0, 0, 0);
        }
    }
#endif
    if (passType != DRAW_PASS_TYPE_TRANSPARENCY)
        RecordBlasInstance(threadIndex, pCommandList, pWorldMat, ppMaterials, numMaterials, ppKeyframes, frameCount);
}

void RaytracingMeshObject::RecordBlasInstance(UINT threadIndex, ID3D12GraphicsCommandList4 *pCommandList,
                                              const Matrix *pWorldMat, IRenderMaterial *const *ppMaterials,
                                              UINT numMaterials, Keyframe **ppKeyframes, UINT frameCount)
{
    RaytracingManager *m_pRaytracingManager = m_pRenderer->GetRaytracingManager();
    DescriptorPool    *pDescriptorPool = m_pRenderer->GetDescriptorPool(threadIndex);
    // ID3D12DescriptorHeap *pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();

    if (m_type == RENDER_ITEM_TYPE_CHAR_OBJ && ppKeyframes != nullptr)
    {
        UpdateSkinnedBLAS(pCommandList, ppKeyframes, frameCount);
    }

    UINT descriptorCount =
        ROOT_ARG_DESCRIPTOR_INDEX_PER_BLAS_COUNT + DESCRIPTOR_COUNT_PER_RAY_FACEGROUP * m_faceGroupCount;

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = {};
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = {};

    pDescriptorPool->Alloc(&cpuHandle, &gpuHandle, descriptorCount);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dest(cpuHandle);
    CD3DX12_CPU_DESCRIPTOR_HANDLE src(m_rootArgDescriptorTable.cpuHandle);

    //  Descriptor Table Per obj
    // | VERTICES |
    // | INDICES0 | ALBEDO0 | NORMAL0 | AO0 | METALLIC_ROUGHNESS0 | EMMISIVE0 | Height0 |
    // | INDICES1 | ALBEDO1 | NORMAL1 | AO1 | METALLIC_ROUGHNESS1 | EMMISIVE1 | Height1 |
    // ...
    m_pD3DDevice->CopyDescriptorsSimple(1, dest, src, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    dest.Offset(m_descriptorSize, 1);
    src.Offset(m_descriptorSize, 1);
    for (UINT i = 0; i < m_faceGroupCount; i++)
    {
        INDEXED_FACE_GROUP *pFace = m_pFaceGroups + i;
        m_pD3DDevice->CopyDescriptorsSimple(1, dest, src, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        dest.Offset(m_descriptorSize);
        src.Offset(m_descriptorSize);

        MATERIAL_HANDLE *pMatHandle = (MATERIAL_HANDLE *)ppMaterials[i];
        pMatHandle->CopyDescriptors(m_pD3DDevice, dest, m_descriptorSize);
        dest.Offset(m_descriptorSize, DESCRIPTOR_INDEX_PER_MATERIAL_COUNT);
    }

    Graphics::LOCAL_ROOT_ARG *pLocalRootArg = (Graphics::LOCAL_ROOT_ARG *)m_pRenderer->FrameAlloc(
        threadIndex, sizeof(Graphics::LOCAL_ROOT_ARG) * m_faceGroupCount);

    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandlePerVB(gpuHandle);
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandlePerFaceGroup(gpuHandle, ROOT_ARG_DESCRIPTOR_INDEX_PER_BLAS_COUNT,
                                                        m_descriptorSize);
    for (UINT i = 0; i < m_faceGroupCount; i++)
    {
        MATERIAL_HANDLE          *pMatHandle = (MATERIAL_HANDLE *)ppMaterials[i];
        Graphics::LOCAL_ROOT_ARG *rootArg = pLocalRootArg + i;
        rootArg->vertices = gpuHandlePerVB;
        rootArg->indices = gpuHandlePerFaceGroup;
        gpuHandlePerFaceGroup.Offset(m_descriptorSize);
        rootArg->textures = gpuHandlePerFaceGroup;
        gpuHandlePerFaceGroup.Offset(m_descriptorSize, DESCRIPTOR_INDEX_PER_MATERIAL_COUNT);
        rootArg->cb.materialIndex = pMatHandle->index;
    }
    m_pRaytracingManager->InsertBLASInstanceAsync(m_bottomLevelAS.pResult, pWorldMat, 0, pLocalRootArg,
                                                  m_faceGroupCount);
}

void RaytracingMeshObject::UpdateSkinnedBLAS(ID3D12GraphicsCommandList4 *pCommandList, Keyframe **ppKeyframes,
                                             UINT frameCount)
{
    DeformingVerticesUAV(pCommandList, ppKeyframes, frameCount);
    BuildBottomLevelAS(pCommandList, m_bottomLevelAS.pScratch, m_bottomLevelAS.pResult, true, m_bottomLevelAS.pResult);
}

BOOL RaytracingMeshObject::BeginCreateMesh(const void *pVertices, UINT numVertices, const Joint *pJoint, UINT numJoint,
                                           UINT numFaceGroup)
{
    BOOL                  result = FALSE;
    ID3D12Device5        *pD3DDeivce = m_pRenderer->GetD3DDevice();
    D3D12ResourceManager *pResourceManager = m_pRenderer->GetResourceManager();

    DASSERT(numFaceGroup <= MAX_FACE_GROUP_COUNT_PER_OBJ);

    // Alloc Descriptor Table
    // | VERTEX BUFFER SRV | INDEX BUFFER SRV0 | INDEX BUFFER SRV1 | ... |
    m_descriptorCountPerDraw = DESCRIPTOR_COUNT_PER_BLAS + numFaceGroup;
    if (!pResourceManager->AllocDescriptorTable(&m_rootArgDescriptorTable, m_descriptorCountPerDraw))
    {
        __debugbreak();
        goto lb_return;
    }
    switch (m_type)
    {
    case RENDER_ITEM_TYPE_MESH_OBJ: {
        if (FAILED(pResourceManager->CreateVertexBuffer(&m_pVertexBuffer, &m_vertexBufferView, sizeof(BasicVertex),
                                                        numVertices, pVertices)))
        {
            __debugbreak();
            goto lb_return;
        }
    }
    break;
    case RENDER_ITEM_TYPE_CHAR_OBJ: {
        if (FAILED(pResourceManager->CreateVertexBuffer(&m_pVertexBuffer, &m_vertexBufferView, sizeof(SkinnedVertex),
                                                        numVertices, pVertices)))
        {
            __debugbreak();
            goto lb_return;
        }
        if (FAILED(pD3DDeivce->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(sizeof(BasicVertex) * numVertices,
                                               D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
                D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_pDeformedVertexBuffer))))
        {
            __debugbreak();
            goto lb_return;
        }
        m_pDeformedVertexBuffer->SetName(L"DeformedVertexBuffer");
    }
    break;
    default: {
        __debugbreak();
        goto lb_return;
    }
    }
    m_pVertexBuffer->SetName(L"MeshObjectVertexBuffer");

    m_vertexCount = numVertices;
    m_maxFaceGroupCount = numFaceGroup;

    m_pFaceGroups = new INDEXED_FACE_GROUP[m_maxFaceGroupCount];
    memset(m_pFaceGroups, 0, sizeof(INDEXED_FACE_GROUP) * m_maxFaceGroupCount);

    m_pBLASGeometries = new D3D12_RAYTRACING_GEOMETRY_DESC[m_maxFaceGroupCount];
    memset(m_pBLASGeometries, 0, sizeof(D3D12_RAYTRACING_GEOMETRY_DESC) * m_maxFaceGroupCount);

    m_pRootArgPerGeometries = new Graphics::LOCAL_ROOT_ARG[m_maxFaceGroupCount];
    memset(m_pRootArgPerGeometries, 0, sizeof(Graphics::LOCAL_ROOT_ARG) * m_maxFaceGroupCount);

    if (numJoint > 0)
    {
        m_pBoneMatrices = new Matrix[numJoint];
        m_pJoints = new Joint[numJoint];
        m_jointCount = numJoint;
        memcpy(m_pJoints, pJoint, sizeof(Joint) * numJoint);
    }
    m_ppMaterials = new IRenderMaterial*[m_maxFaceGroupCount];

    result = TRUE;
lb_return:
    return result;
}

BOOL RaytracingMeshObject::InsertFaceGroup(const UINT *pIndices, UINT numTriangles, IRenderMaterial *pMaterial)
{
    BOOL                  result = FALSE;
    ID3D12Device5        *pD3DDeivce = m_pRenderer->GetD3DDevice();
    D3D12ResourceManager *resourceManager = m_pRenderer->GetResourceManager();
    UINT                  srvDescriptorSize = resourceManager->GetDescriptorSize();

    ID3D12Resource         *pIndexBuffer = nullptr;
    D3D12_INDEX_BUFFER_VIEW IndexBufferView = {};

    if (m_faceGroupCount >= m_maxFaceGroupCount)
    {
        __debugbreak();
        goto lb_return;
    }
    if (FAILED(resourceManager->CreateIndexBuffer(&pIndexBuffer, &IndexBufferView, numTriangles * 3, pIndices)))
    {
        __debugbreak();
        goto lb_return;
    }

    INDEXED_FACE_GROUP *pFaceGroup = m_pFaceGroups + m_faceGroupCount;
    pFaceGroup->pIndexBuffer = pIndexBuffer;
    pFaceGroup->IndexBufferView = IndexBufferView;
    pFaceGroup->numTriangles = numTriangles;

    ID3D12Resource *pVertices = (m_type == RENDER_ITEM_TYPE_CHAR_OBJ) ? m_pDeformedVertexBuffer : m_pVertexBuffer;
    AddBLASGeometry(m_faceGroupCount, pVertices, 0, m_vertexCount, sizeof(BasicVertex), pIndexBuffer, 0,
                    numTriangles * 3, 0, 0);
    m_ppMaterials[m_faceGroupCount] = pMaterial;

    m_faceGroupCount++;

    result = TRUE;
lb_return:
    return result;
}

void RaytracingMeshObject::EndCreateMesh() { m_pRenderer->EndCreateMesh(this); }

void RaytracingMeshObject::EndCreateMesh(ID3D12GraphicsCommandList4 *pCommandList)
{
    CreateSkinningBufferSRVs();
    CreateRootArgsSRV();
    CreateBottomLevelAS(pCommandList);
}

void RaytracingMeshObject::AddBLASGeometry(UINT faceGroupIndex, ID3D12Resource *vertexBuffer,
                                           UINT64 vertexOffsetInBytes, uint32_t vertexCount, UINT vertexSizeInBytes,
                                           ID3D12Resource *indexBuffer, UINT64 indexOffsetInBytes, uint32_t indexCount,
                                           ID3D12Resource *transformBuffer, UINT64 transformOffsetInBytes,
                                           bool isOpaque)
{
    // Create the DX12 descriptor representing the input data, assumed to be
    // opaque triangles, with 3xf32 vertex coordinates and 32-bit indices
    D3D12_RAYTRACING_GEOMETRY_DESC *descriptor = m_pBLASGeometries + faceGroupIndex;
    descriptor->Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    descriptor->Triangles.VertexBuffer.StartAddress = vertexBuffer->GetGPUVirtualAddress() + vertexOffsetInBytes;
    descriptor->Triangles.VertexBuffer.StrideInBytes = vertexSizeInBytes;
    descriptor->Triangles.VertexCount = vertexCount;
    descriptor->Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
    descriptor->Triangles.IndexBuffer = indexBuffer ? (indexBuffer->GetGPUVirtualAddress() + indexOffsetInBytes) : 0;
    descriptor->Triangles.IndexFormat = indexBuffer ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_UNKNOWN;
    descriptor->Triangles.IndexCount = indexCount;
    descriptor->Triangles.Transform3x4 =
        transformBuffer ? (transformBuffer->GetGPUVirtualAddress() + transformOffsetInBytes) : 0;
    descriptor->Flags = isOpaque ? D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE : D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
    // descriptor->Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
}

void RaytracingMeshObject::DeformingVerticesUAV(ID3D12GraphicsCommandList4 *pCommandList, Keyframe **ppKeyframes,
                                                UINT frameCount)
{
    if (m_type != RENDER_ITEM_TYPE_CHAR_OBJ)
    {
        __debugbreak();
        return;
    }

    ConstantBufferPool *pSkinnedConstantBufferPool =
        m_pRenderer->GetConstantBufferPool(CONSTANT_BUFFER_TYPE_SKINNED, 0);
    DescriptorPool       *pDescriptorPool = m_pRenderer->GetDescriptorPool(0);
    ID3D12DescriptorHeap *pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();

    // CB Data Binding
    CB_CONTAINER *pSkinnedCB = nullptr;
    pSkinnedCB = pSkinnedConstantBufferPool->Alloc();
    memcpy(pSkinnedCB, m_pBoneMatrices, sizeof(Matrix) * m_jointCount);

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = {};
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = {};

    pDescriptorPool->Alloc(&cpuHandle, &gpuHandle, 3);

    CD3DX12_CPU_DESCRIPTOR_HANDLE dest(cpuHandle);
    m_pD3DDevice->CopyDescriptorsSimple(SKINNING_DESCRIPTOR_INDEX_COUNT, dest, m_skinningDescriptors.cpuHandle,
                                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    dest.Offset(m_descriptorSize, SKINNING_DESCRIPTOR_INDEX_COUNT);
    m_pD3DDevice->CopyDescriptorsSimple(1, dest, pSkinnedCB->CBVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    pCommandList->ResourceBarrier(
        1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pVertexBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
                                                 D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE));
    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pDeformedVertexBuffer,
                                                                           D3D12_RESOURCE_STATE_COMMON,
                                                                           D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    pCommandList->SetPipelineState(Graphics::deformingVertexPSO);
    pCommandList->SetComputeRootSignature(Graphics::deformingVertexRS);
    pCommandList->SetDescriptorHeaps(1, &pDescriptorHeap);

    pCommandList->SetComputeRootDescriptorTable(0, gpuHandle);
    pCommandList->Dispatch(UINT(ceil(m_vertexCount / 1024.0f)), 1, 1);

    /*D3D12_RESOURCE_BARRIER uavBarrier;
    uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    uavBarrier.UAV.pResource = m_pVertexUAVBuffer;
    uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    pCommandList->ResourceBarrier(1, &uavBarrier);*/
    pCommandList->ResourceBarrier(
        1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pVertexBuffer, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
                                                 D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pDeformedVertexBuffer,
                                                                           D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                                                           D3D12_RESOURCE_STATE_COMMON));
}

BOOL RaytracingMeshObject::CreateBottomLevelAS(ID3D12GraphicsCommandList4 *pCommandList)
{
    BOOL result = FALSE;

    ID3D12Device5  *pDevice = m_pD3DDevice;
    ID3D12Resource *pScratch = nullptr;
    ID3D12Resource *pResult = nullptr;

    m_BLASFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    // Copy Vertex Buffer -> Vertex Buffer UAV (Deformed Vertice)
    if (m_type == RENDER_ITEM_TYPE_CHAR_OBJ)
    {
        m_BLASFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE |
                      D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
    }

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS prebuildDesc;
    prebuildDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    prebuildDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    prebuildDesc.NumDescs = m_faceGroupCount;
    prebuildDesc.pGeometryDescs = m_pBLASGeometries;
    prebuildDesc.Flags = m_BLASFlags;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};

    pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildDesc, &info);

    UINT64 scratchSizeInBytes = ROUND_UP(info.ScratchDataSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
    UINT64 resultSizeInBytes = ROUND_UP(info.ResultDataMaxSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

    // Create Scratch Buffer
    if (FAILED(pDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(scratchSizeInBytes * 2, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
            D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pScratch))))
    {
        __debugbreak();
        goto lb_return;
    }
    // Create BLAS Result Buffer
    if (FAILED(pDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(resultSizeInBytes * 2, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
            D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, nullptr, IID_PPV_ARGS(&pResult))))
    {
        __debugbreak();
        goto lb_return;
    }

    m_BLASResultSizeInBytes = resultSizeInBytes;
    m_BLASScratchSizeInBytes = scratchSizeInBytes;
    BuildBottomLevelAS(pCommandList, pScratch, pResult);

    m_bottomLevelAS.pScratch = pScratch;
    m_bottomLevelAS.pResult = pResult;

    result = TRUE;
lb_return:
    return result;
}

BOOL RaytracingMeshObject::BuildBottomLevelAS(
    ID3D12GraphicsCommandList4 *commandList,   /// Command list on which the build will be enqueued
    ID3D12Resource             *scratchBuffer, /// Scratch buffer used by the builder to
                                               /// store temporary data
    ID3D12Resource             *resultBuffer,  /// Result buffer storing the acceleration structure
    bool                        isUpdate,      /// If true, simply refit the existing acceleration structure
    ID3D12Resource             *previousResult /// Optional previous acceleration structure, used
                                               /// if an iterative update is requested
)
{
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS flags = m_BLASFlags;
    if (flags & D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE && isUpdate)
    {
        flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
    }
    // Sanity checks
    if (!(m_BLASFlags & D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE) && isUpdate)
    {
        __debugbreak();
    }
    if (isUpdate && previousResult == nullptr)
    {
        __debugbreak();
    }
    if (m_BLASResultSizeInBytes == 0 || m_BLASScratchSizeInBytes == 0)
    {
        __debugbreak();
    }

    // Create a descriptor of the requested builder work, to generate a
    // bottom-level AS from the input parameters
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc;
    buildDesc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    buildDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    buildDesc.Inputs.NumDescs = m_faceGroupCount;
    buildDesc.Inputs.pGeometryDescs = m_pBLASGeometries;
    buildDesc.DestAccelerationStructureData = {resultBuffer->GetGPUVirtualAddress()};
    buildDesc.ScratchAccelerationStructureData = {scratchBuffer->GetGPUVirtualAddress()};
    buildDesc.SourceAccelerationStructureData = previousResult ? previousResult->GetGPUVirtualAddress() : 0;
    buildDesc.Inputs.Flags = flags;

    // Build the AS
    commandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

    // Wait for the builder to complete by setting a barrier on the resulting
    // buffer. This is particularly important as the construction of the top-level
    // hierarchy may be called right afterwards, before executing the command
    // list.
    D3D12_RESOURCE_BARRIER uavBarrier;
    uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    uavBarrier.UAV.pResource = resultBuffer;
    uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    commandList->ResourceBarrier(1, &uavBarrier);

    return TRUE;
}

void RaytracingMeshObject::CreateSkinningBufferSRVs()
{
    if (m_type == RENDER_ITEM_TYPE_MESH_OBJ)
        return;

    D3D12ResourceManager *resourceManager = m_pRenderer->GetResourceManager();

    // | VertexBuffer(t0) | DeformedVertexBuffer(u0) |
    resourceManager->AllocDescriptorTable(&m_skinningDescriptors, SKINNING_DESCRIPTOR_INDEX_COUNT);
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_skinningDescriptors.cpuHandle);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    srvDesc.Buffer.NumElements = m_vertexCount;
    srvDesc.Buffer.StructureByteStride = sizeof(SkinnedVertex);
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    m_pD3DDevice->CreateShaderResourceView(m_pVertexBuffer, &srvDesc, cpuHandle);
    cpuHandle.Offset(m_descriptorSize);

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.Buffer.CounterOffsetInBytes = 0;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
    uavDesc.Buffer.NumElements = m_vertexCount;
    uavDesc.Buffer.StructureByteStride = sizeof(BasicVertex);
    m_pD3DDevice->CreateUnorderedAccessView(m_pDeformedVertexBuffer, nullptr, &uavDesc, cpuHandle);
}

// | Vertices | Indices0 | Diffuse Tex0 | Indices1 | Diffuse Tex1 | ... |
void RaytracingMeshObject::CreateRootArgsSRV()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_rootArgDescriptorTable.cpuHandle);

    // Create Vertex Buffer SRV
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    srvDesc.Buffer.NumElements = m_vertexCount;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.StructureByteStride = sizeof(BasicVertex);
    ID3D12Resource *pResource = (m_type == RENDER_ITEM_TYPE_MESH_OBJ) ? m_pVertexBuffer : m_pDeformedVertexBuffer;
    m_pD3DDevice->CreateShaderResourceView(pResource, &srvDesc, cpuHandle);
    cpuHandle.Offset(ROOT_ARG_DESCRIPTOR_INDEX_PER_BLAS_COUNT, m_descriptorSize);

    for (UINT i = 0; i < m_faceGroupCount; i++)
    {
        // Create Index Buffer & Texture SRV
        INDEXED_FACE_GROUP *pFace = m_pFaceGroups + i;

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Buffer.FirstElement = 0;
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
        srvDesc.Buffer.StructureByteStride = sizeof(UINT);
        srvDesc.Buffer.NumElements = pFace->numTriangles * 3;
        m_pD3DDevice->CreateShaderResourceView(pFace->pIndexBuffer, &srvDesc, cpuHandle);
        cpuHandle.Offset(m_descriptorSize);
    }
}

void RaytracingMeshObject::CleanupMesh()
{
    // Cleanup BLAS
    {
        if (m_bottomLevelAS.pInstanceDesc)
        {
            m_bottomLevelAS.pInstanceDesc->Release();
            m_bottomLevelAS.pInstanceDesc = nullptr;
        }
        if (m_bottomLevelAS.pResult)
        {
            m_bottomLevelAS.pResult->Release();
            m_bottomLevelAS.pResult = nullptr;
        }
        if (m_bottomLevelAS.pScratch)
        {
            m_bottomLevelAS.pScratch->Release();
            m_bottomLevelAS.pScratch = nullptr;
        }
    }

    if (m_pBLASGeometries)
    {
        delete[] m_pBLASGeometries;
        m_pBLASGeometries = nullptr;
    }

    if (m_pRootArgPerGeometries)
    {
        delete[] m_pRootArgPerGeometries;
        m_pRootArgPerGeometries = nullptr;
    }

    if (m_pFaceGroups)
    {
        for (UINT i = 0; i < m_faceGroupCount; i++)
        {
            if (m_pFaceGroups[i].pIndexBuffer)
            {
                m_pFaceGroups[i].pIndexBuffer->Release();
                m_pFaceGroups[i].pIndexBuffer = nullptr;
            }
        }
        delete[] m_pFaceGroups;
        m_pFaceGroups = nullptr;
    }

    if (m_pVertexBuffer)
    {
        m_pVertexBuffer->Release();
        m_pVertexBuffer = nullptr;
    }

    if (m_pDeformedVertexBuffer)
    {
        m_pDeformedVertexBuffer->Release();
        m_pDeformedVertexBuffer = nullptr;
    }

    if (m_skinningDescriptors.descriptorCount > 0)
    {
        m_pRenderer->GetResourceManager()->DeallocDescriptorTable(&m_skinningDescriptors);
        m_skinningDescriptors = {};
    }
    if (m_ppMaterials)
    {
        delete[] m_ppMaterials;
        m_ppMaterials = nullptr;
    }
    if (m_pJoints)
    {
        delete[] m_pJoints;
        m_pJoints = nullptr;
    }
    if (m_pBoneMatrices)
    {
        delete[] m_pBoneMatrices;
        m_pBoneMatrices = nullptr;
    }
}

void RaytracingMeshObject::Cleanup() { CleanupMesh(); }

RaytracingMeshObject::~RaytracingMeshObject() { Cleanup(); }

HRESULT __stdcall RaytracingMeshObject::QueryInterface(REFIID riid, void **ppvObject) { return E_NOTIMPL; }

ULONG __stdcall RaytracingMeshObject::AddRef(void) { return ++m_refCount; }

ULONG __stdcall RaytracingMeshObject::Release(void)
{
    ULONG newRefCount = --m_refCount;
    if (newRefCount == 0)
    {
        m_pRenderer->WaitForGPU();
        delete this;
        return 0;
    }
    return newRefCount;
}
