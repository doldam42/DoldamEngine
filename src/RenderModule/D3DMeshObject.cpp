#include "pch.h"

#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <d3dx12.h>

#include "ConstantBuffers.h"

#include "ConstantBufferPool.h"
#include "Cubemap.h"
#include "D3D12Renderer.h"
#include "D3D12ResourceManager.h"
#include "DescriptorPool.h"

#include "ShaderTable.h"

#include "D3DMeshObject.h"

// 레이트레이싱 SRV 디스크립터 구조
// | VertexBuffer(SRV) | IndexBuffer0(SRV) | DiffuseTex0(SRV) | ... |
BOOL D3DMeshObject::CreateDescriptorTable()
{
    BOOL                  result = FALSE;
    ID3D12Device         *pD3DDevice = m_pRenderer->INL_GetD3DDevice();
    D3D12ResourceManager *pResourceManager = m_pRenderer->INL_GetResourceManager();

    if (!pResourceManager->AllocDescriptorTable(&m_rootArgDescriptorTable, MAX_DESCRIPTOR_COUNT_PER_BLAS))
    {
        __debugbreak();
        goto lb_return;
    }

    result = TRUE;
lb_return:
    return result;
}

BOOL D3DMeshObject::Initialize(D3D12Renderer *pRenderer, RENDER_ITEM_TYPE type)
{
    ID3D12Device *pDevice = pRenderer->INL_GetD3DDevice();
    m_descriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    m_pRenderer = pRenderer;
    m_pD3DDevice = pDevice;
    m_type = type;

#ifdef USE_RAYTRACING
    CreateDescriptorTable();
#endif
    return TRUE;
}

void D3DMeshObject::Draw(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList, const Matrix *pWorldMat,
                         const Matrix *pBoneMats, UINT numBones, FILL_MODE fillMode, UINT numInstance,
                         DRAW_PASS_TYPE passType)
{
    switch (passType)
    {
    case DRAW_PASS_TYPE_DEFAULT:
    case DRAW_PASS_TYPE_NON_OPAQUE:
        Render(threadIndex, pCommandList, pWorldMat, pBoneMats, numBones, fillMode, numInstance);
        // RenderNormal(threadIndex, pCommandList, pWorldMat, pBoneMats, numBones, fillMode, numInstance);
        break;
    case DRAW_PASS_TYPE_SHADOW:
        RenderShadowMap(threadIndex, pCommandList, pWorldMat, pBoneMats, numBones, fillMode, numInstance);
        break;
    default:
        break;
    }
}

void D3DMeshObject::Render(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList, const Matrix *pWorldMat,
                         const Matrix *pBoneMats, UINT numBones, FILL_MODE fillMode, UINT numInstance)
{
    DescriptorPool       *pDescriptorPool = m_pRenderer->INL_GetDescriptorPool(threadIndex);
    ID3D12DescriptorHeap *pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = {};
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = {};
    pDescriptorPool->Alloc(&cpuHandle, &gpuHandle, m_descriptorCountPerDraw);

    UpdateDescriptorTablePerObj(cpuHandle, threadIndex, pWorldMat, numInstance, pBoneMats, numBones);
    UpdateDescriptorTablePerFaceGroup(cpuHandle, threadIndex);

    // set RootSignature
    ID3D12RootSignature *pSignature = Graphics::GetRS(m_type);
    ID3D12PipelineState *pPipelineState = Graphics::GetPSO(m_type, m_passType, fillMode);
    pCommandList->SetGraphicsRootSignature(pSignature);
    ID3D12DescriptorHeap *ppHeaps[] = {pDescriptorHeap};
    pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    pCommandList->SetPipelineState(pPipelineState);
    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    CD3DX12_GPU_DESCRIPTOR_HANDLE _gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(gpuHandle);
    pCommandList->SetGraphicsRootDescriptorTable(0, m_pRenderer->GetGlobalDescriptorHandle(threadIndex));
    pCommandList->SetGraphicsRootDescriptorTable(1, _gpuHandle);
    if (m_type == RENDER_ITEM_TYPE_MESH_OBJ)
    {
        _gpuHandle.Offset(m_descriptorSize, DESCRIPTOR_COUNT_PER_STATIC_OBJ);
    }
    else
    {
        _gpuHandle.Offset(m_descriptorSize, DESCRIPTOR_COUNT_PER_DYNAMIC_OBJ);
    }

    for (UINT i = 0; i < m_faceGroupCount; i++)
    {
        pCommandList->SetGraphicsRootDescriptorTable(2, _gpuHandle);
        _gpuHandle.Offset(m_descriptorSize, DESCRIPTOR_INDEX_PER_FACE_GROUP_COUNT);

        INDEXED_FACE_GROUP *pFaceGroup = m_pFaceGroups + i;
        pCommandList->IASetIndexBuffer(&pFaceGroup->IndexBufferView);
        pCommandList->DrawIndexedInstanced(pFaceGroup->numTriangles * 3, numInstance, 0, 0, 0);
    }
}

void D3DMeshObject::RenderShadowMap(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList, const Matrix *pWorldMat,
                                  const Matrix *pBoneMats, UINT numBones, FILL_MODE fillMode, UINT numInstance)
{
    DescriptorPool       *pDescriptorPool = m_pRenderer->INL_GetDescriptorPool(threadIndex);
    ID3D12DescriptorHeap *pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();
    
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = {};
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = {};
    pDescriptorPool->Alloc(&cpuHandle, &gpuHandle, m_descriptorCountPerDraw);

    UpdateDescriptorTablePerObj(cpuHandle, threadIndex, pWorldMat, numInstance, pBoneMats, numBones);
    
    ID3D12DescriptorHeap *ppHeaps[] = {pDescriptorHeap};
    pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    if (m_type == RENDER_ITEM_TYPE_MESH_OBJ)
    {
        pCommandList->SetGraphicsRootSignature(Graphics::depthOnlyBasicRS);
    }
    else
    {
        pCommandList->SetGraphicsRootSignature(Graphics::depthOnlySkinnedRS);
    }
    pCommandList->SetPipelineState(Graphics::GetPSO(m_type, DRAW_PASS_TYPE_SHADOW, fillMode));
    
    CD3DX12_GPU_DESCRIPTOR_HANDLE _gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(gpuHandle);
    pCommandList->SetGraphicsRootDescriptorTable(0, m_pRenderer->GetShadowGlobalDescriptorHandle(threadIndex));
    pCommandList->SetGraphicsRootDescriptorTable(1, _gpuHandle);
    
    for (UINT i = 0; i < m_faceGroupCount; i++)
    {
        INDEXED_FACE_GROUP *pFaceGroup = m_pFaceGroups + i;
        pCommandList->IASetIndexBuffer(&pFaceGroup->IndexBufferView);
        pCommandList->DrawIndexedInstanced(pFaceGroup->numTriangles * 3, numInstance, 0, 0, 0);
    }
}

void D3DMeshObject::RenderNormal(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList, const Matrix *pWorldMat,
                                 const Matrix *pBoneMats, UINT numBones, FILL_MODE fillMode, UINT numInstance)
{
    DescriptorPool       *pDescriptorPool = m_pRenderer->INL_GetDescriptorPool(threadIndex);
    ID3D12DescriptorHeap *pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = {};
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = {};
    pDescriptorPool->Alloc(&cpuHandle, &gpuHandle, m_descriptorCountPerDraw);

    UpdateDescriptorTablePerObj(cpuHandle, threadIndex, pWorldMat, numInstance, pBoneMats, numBones);

    ID3D12DescriptorHeap *ppHeaps[] = {pDescriptorHeap};
    pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
    pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    if (m_type == RENDER_ITEM_TYPE_MESH_OBJ)
    {
        pCommandList->SetGraphicsRootSignature(Graphics::depthOnlyBasicRS);
        pCommandList->SetPipelineState(Graphics::drawNormalPSO);
    }
    else
    {
        pCommandList->SetGraphicsRootSignature(Graphics::depthOnlySkinnedRS);
        pCommandList->SetPipelineState(Graphics::drawSkinnedNormalPSO);
    }

    CD3DX12_GPU_DESCRIPTOR_HANDLE _gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(gpuHandle);
    pCommandList->SetGraphicsRootDescriptorTable(0, m_pRenderer->GetGlobalDescriptorHandle(threadIndex));
    pCommandList->SetGraphicsRootDescriptorTable(1, _gpuHandle);

    pCommandList->DrawInstanced(m_vertexCount, numInstance, 0, 0);
}

/*
 *  Descriptor Table Per Obj - Offset : 0
 *  Basic Mesh :   | World TM |
 *  Skinned Mesh:  | World TM | Bone Matrices |
 */
void D3DMeshObject::UpdateDescriptorTablePerObj(D3D12_CPU_DESCRIPTOR_HANDLE descriptorTable, UINT threadIndex,
                                                const Matrix *pWorldMat, UINT numInstance, const Matrix *pBoneMats,
                                                UINT numBones)
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

    if (m_type == RENDER_ITEM_TYPE_CHAR_OBJ)
    {
        dest.Offset(m_descriptorSize);
        pSkinnedConstantBufferPool = m_pRenderer->GetConstantBufferPool(CONSTANT_BUFFER_TYPE_SKINNED, threadIndex);
        pSkinnedCB = pSkinnedConstantBufferPool->Alloc();
        Matrix *pBoneTMs = (Matrix *)pSkinnedCB->pSystemMemAddr;
        for (UINT i = 0; i < numBones; i++)
        {
            Matrix *pBoneTM = pBoneTMs + i;
            memcpy(pBoneTM, &pBoneMats[i].Transpose(), sizeof(Matrix));
        }
        m_pD3DDevice->CopyDescriptorsSimple(1, dest, pSkinnedCB->CBVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }
}

/*
 *  Descriptor Table Per FaceGroup - Offset : 0
 *  | Material | Textures |
 */
void D3DMeshObject::UpdateDescriptorTablePerFaceGroup(D3D12_CPU_DESCRIPTOR_HANDLE descriptorTable, UINT threadIndex)
{
    CB_CONTAINER       *pGeomCBs;
    ConstantBufferPool *pGeometryConstantBufferPool =
        m_pRenderer->GetConstantBufferPool(CONSTANT_BUFFER_TYPE_GEOMETRY, threadIndex);
    
    CD3DX12_CPU_DESCRIPTOR_HANDLE dest(descriptorTable);
    if (m_type == RENDER_ITEM_TYPE_MESH_OBJ)
        dest.Offset(m_descriptorSize, DESCRIPTOR_COUNT_PER_STATIC_OBJ);
    else       
        dest.Offset(m_descriptorSize, DESCRIPTOR_COUNT_PER_DYNAMIC_OBJ);

    pGeomCBs = pGeometryConstantBufferPool->Alloc(m_faceGroupCount);
    for (UINT i = 0; i < m_faceGroupCount; i++)
    {
        INDEXED_FACE_GROUP *pFaceGroup = m_pFaceGroups + i;
        CB_CONTAINER       *pGeomCB = pGeomCBs + i;

        MATERIAL_HANDLE *pMatHandle = pFaceGroup->pMaterialHandle;
        TEXTURE_HANDLE  *pAlbedoTexHandle = pFaceGroup->pAlbedoTexHandle;
        TEXTURE_HANDLE  *pAOTexHandle = pFaceGroup->pAOTexHandle;
        TEXTURE_HANDLE  *pEmissiveTexHandle = pFaceGroup->pEmissiveTexHandle;
        TEXTURE_HANDLE  *pMetallicRoughnessTexHandle = pFaceGroup->pMetallicRoughnessTexHandle;
        TEXTURE_HANDLE  *pNormalTexHandle = pFaceGroup->pNormalTexHandle;

        GeometryConstants *pGeometry = (GeometryConstants *)pGeomCB->pSystemMemAddr;
        pGeometry->materialIndex = pMatHandle->index;
        pGeometry->useTexture = !pAlbedoTexHandle ? FALSE : TRUE;

        CD3DX12_CPU_DESCRIPTOR_HANDLE materialCBV(dest, m_descriptorSize, DESCRIPTOR_INDEX_PER_FACE_GROUP_MATERIAL);
        CD3DX12_CPU_DESCRIPTOR_HANDLE albedoSRV(dest, m_descriptorSize, DESCRIPTOR_INDEX_PER_FACE_GROUP_TEX_ALBEDO);
        CD3DX12_CPU_DESCRIPTOR_HANDLE normalSRV(dest, m_descriptorSize, DESCRIPTOR_INDEX_PER_FACE_GROUP_TEX_NORMAL);
        CD3DX12_CPU_DESCRIPTOR_HANDLE aoSRV(dest, m_descriptorSize, DESCRIPTOR_INDEX_PER_FACE_GROUP_TEX_AO);
        CD3DX12_CPU_DESCRIPTOR_HANDLE metallicRoughnessSRV(dest, m_descriptorSize,
                                                           DESCRIPTOR_INDEX_PER_FACE_GROUP_TEX_METALLIC_ROUGHNESS);
        CD3DX12_CPU_DESCRIPTOR_HANDLE emissiveSRV(dest, m_descriptorSize, DESCRIPTOR_INDEX_PER_FACE_GROUP_TEX_EMISSIVE);

        m_pD3DDevice->CopyDescriptorsSimple(1, materialCBV, pGeomCB->CBVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        if (pAlbedoTexHandle)
        {
            m_pD3DDevice->CopyDescriptorsSimple(1, albedoSRV, pAlbedoTexHandle->srv.cpuHandle,
                                           D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }
        if (pNormalTexHandle)
        {
            m_pD3DDevice->CopyDescriptorsSimple(1, normalSRV, pNormalTexHandle->srv.cpuHandle,
                                           D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }
        if (pAOTexHandle)
        {
            m_pD3DDevice->CopyDescriptorsSimple(1, aoSRV, pAOTexHandle->srv.cpuHandle,
                                           D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }
        if (pMetallicRoughnessTexHandle)
        {
            m_pD3DDevice->CopyDescriptorsSimple(1, metallicRoughnessSRV, pMetallicRoughnessTexHandle->srv.cpuHandle,
                                           D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }
        if (pEmissiveTexHandle)
        {
            m_pD3DDevice->CopyDescriptorsSimple(1, emissiveSRV, pEmissiveTexHandle->srv.cpuHandle,
                                           D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }
        dest.Offset(m_descriptorSize, DESCRIPTOR_INDEX_PER_FACE_GROUP_COUNT);
    }
}

void D3DMeshObject::UpdateSkinnedBLAS(ID3D12GraphicsCommandList4 *pCommandList, const Matrix *pBoneMats, UINT numBones)
{
    DeformingVerticesUAV(pCommandList, pBoneMats, numBones);
    BuildBottomLevelAS(pCommandList, m_bottomLevelAS.pScratch, m_bottomLevelAS.pResult, true, m_bottomLevelAS.pResult);
}

Graphics::LOCAL_ROOT_ARG *D3DMeshObject::GetRootArgs()
{
    ID3D12Device5        *pDevice = m_pRenderer->INL_GetD3DDevice();
    DescriptorPool       *pDescriptorPool = m_pRenderer->INL_GetDescriptorPool(0);
    ID3D12DescriptorHeap *pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();

    UINT descriptorCount =
        ROOT_ARG_DESCRIPTOR_INDEX_PER_BLAS_COUNT + DESCRIPTOR_COUNT_PER_RAY_GEOMETRY * m_faceGroupCount;

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = {};
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = {};

    pDescriptorPool->Alloc(&cpuHandle, &gpuHandle, descriptorCount);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dest(cpuHandle);
    CD3DX12_CPU_DESCRIPTOR_HANDLE src(m_rootArgDescriptorTable.cpuHandle);

    pDevice->CopyDescriptorsSimple(descriptorCount, dest, src, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandlePerVB(gpuHandle);
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandlePerGeom(gpuHandle, ROOT_ARG_DESCRIPTOR_INDEX_PER_BLAS_COUNT,
                                                   m_descriptorSize);
    for (UINT i = 0; i < m_faceGroupCount; i++)
    {
        Graphics::LOCAL_ROOT_ARG *rootArg = m_pRootArgPerGeometries + i;
        rootArg->vertices = gpuHandlePerVB;
        rootArg->indices = gpuHandlePerGeom;
        gpuHandlePerGeom.Offset(m_descriptorSize);
        rootArg->diffuseTex = gpuHandlePerGeom;
        gpuHandlePerGeom.Offset(m_descriptorSize);
    }

    return m_pRootArgPerGeometries;
}

BOOL D3DMeshObject::BeginCreateMesh(const void *pVertices, UINT numVertices, UINT numFaceGroup, const wchar_t *path)
{
    BOOL                  result = FALSE;
    ID3D12Device5        *pD3DDeivce = m_pRenderer->INL_GetD3DDevice();
    D3D12ResourceManager *resourceManager = m_pRenderer->INL_GetResourceManager();

    if (path)
    {
        memset(m_basePath, 0, sizeof(m_basePath));
        wcscpy_s(m_basePath, path);
    }

    if (numFaceGroup > MAX_FACE_GROUP_COUNT_PER_OBJ)
        __debugbreak();

    switch (m_type)
    {
    case RENDER_ITEM_TYPE_MESH_OBJ: {
        if (FAILED(resourceManager->CreateVertexBuffer(&m_pVertexBuffer, &m_vertexBufferView, sizeof(BasicVertex),
                                                       numVertices, pVertices)))
        {
            __debugbreak();
            goto lb_return;
        }
    }
    break;
    case RENDER_ITEM_TYPE_CHAR_OBJ: {
        if (FAILED(resourceManager->CreateVertexBuffer(&m_pVertexBuffer, &m_vertexBufferView, sizeof(SkinnedVertex),
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

    // #DXR
    m_pBLASGeometries = new D3D12_RAYTRACING_GEOMETRY_DESC[m_maxFaceGroupCount];
    memset(m_pBLASGeometries, 0, sizeof(D3D12_RAYTRACING_GEOMETRY_DESC) * m_maxFaceGroupCount);
    m_pRootArgPerGeometries = new Graphics::LOCAL_ROOT_ARG[m_maxFaceGroupCount];
    memset(m_pRootArgPerGeometries, 0, sizeof(Graphics::LOCAL_ROOT_ARG) * m_maxFaceGroupCount);

    m_descriptorCountPerDraw =
        (m_type == RENDER_ITEM_TYPE_MESH_OBJ) ? DESCRIPTOR_COUNT_PER_STATIC_OBJ : DESCRIPTOR_COUNT_PER_DYNAMIC_OBJ;
    m_descriptorCountPerDraw += numFaceGroup * DESCRIPTOR_INDEX_PER_FACE_GROUP_COUNT;

    result = TRUE;
lb_return:
    return result;
}

BOOL D3DMeshObject::InsertFaceGroup(const UINT *pIndices, UINT numTriangles, const Material *pInMaterial)
{
    wchar_t path[MAX_PATH];

    BOOL                  result = FALSE;
    ID3D12Device5        *pD3DDeivce = m_pRenderer->INL_GetD3DDevice();
    D3D12ResourceManager *resourceManager = m_pRenderer->INL_GetResourceManager();
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

    TEXTURE_HANDLE *pAlbedoTexHandle = nullptr;
    TEXTURE_HANDLE *pNormalTexHandle = nullptr;
    TEXTURE_HANDLE *pAOTexHandle = nullptr;
    TEXTURE_HANDLE *pMetallicRoughnessTexHandle = nullptr;
    TEXTURE_HANDLE *pEmissiveTexHandle = nullptr;

    // Albedo
    memset(path, 0, sizeof(path));
    wcscpy_s(path, m_basePath);
    wcscat_s(path, pInMaterial->albedoTextureName);
    pAlbedoTexHandle = (TEXTURE_HANDLE *)m_pRenderer->CreateTextureFromFile(path);

    // Normal
    memset(path, 0, sizeof(path));
    wcscpy_s(path, m_basePath);
    wcscat_s(path, pInMaterial->normalTextureName);
    pNormalTexHandle = (TEXTURE_HANDLE *)m_pRenderer->CreateTextureFromFile(path);

    // AO
    memset(path, 0, sizeof(path));
    wcscpy_s(path, m_basePath);
    wcscat_s(path, pInMaterial->aoTextureName);
    pAOTexHandle = (TEXTURE_HANDLE *)m_pRenderer->CreateTextureFromFile(path);

    // Emissive
    memset(path, 0, sizeof(path));
    wcscpy_s(path, m_basePath);
    wcscat_s(path, pInMaterial->emissiveTextureName);
    pEmissiveTexHandle = (TEXTURE_HANDLE *)m_pRenderer->CreateTextureFromFile(path);

    // Metallic-Roughness
    WCHAR metallicPath[MAX_PATH];
    WCHAR roughnessPath[MAX_PATH];
    memset(metallicPath, 0, sizeof(metallicPath));
    memset(roughnessPath, 0, sizeof(roughnessPath));

    wcscpy_s(metallicPath, m_basePath);
    wcscpy_s(roughnessPath, m_basePath);

    wcscat_s(metallicPath, pInMaterial->metallicTextureName);
    wcscat_s(roughnessPath, pInMaterial->roughnessTextureName);

    pMetallicRoughnessTexHandle =
        static_cast<TEXTURE_HANDLE *>(m_pRenderer->CreateMetallicRoughnessTexture(metallicPath, roughnessPath));

    if (!pAlbedoTexHandle)
    {
        pAlbedoTexHandle = m_pRenderer->GetDefaultTex();
    }
    if (!pNormalTexHandle)
    {
        pNormalTexHandle = m_pRenderer->GetDefaultTex();
    }
    if (!pAOTexHandle)
    {
        pAOTexHandle = m_pRenderer->GetDefaultTex();
    }
    if (!pEmissiveTexHandle)
    {
        pEmissiveTexHandle = m_pRenderer->GetDefaultTex();
    }
    if (!pMetallicRoughnessTexHandle)
    {
        pMetallicRoughnessTexHandle = m_pRenderer->GetDefaultTex();
    }

    m_passType = (pInMaterial->opacityFactor < 0.99f) ? DRAW_PASS_TYPE_NON_OPAQUE : DRAW_PASS_TYPE_DEFAULT;

    pFaceGroup->pMaterialHandle = (MATERIAL_HANDLE *)m_pRenderer->CreateMaterialHandle(pInMaterial);
    pFaceGroup->pAlbedoTexHandle = pAlbedoTexHandle;
    pFaceGroup->pNormalTexHandle = pNormalTexHandle;
    pFaceGroup->pAOTexHandle = pAOTexHandle;
    pFaceGroup->pMetallicRoughnessTexHandle = pMetallicRoughnessTexHandle;
    pFaceGroup->pEmissiveTexHandle = pEmissiveTexHandle;

    // root arg per geometry
    {
        m_pRootArgPerGeometries[m_faceGroupCount].cb.useTexture = wcslen(pInMaterial->albedoTextureName) == 0 ? 1 : 0;
        m_pRootArgPerGeometries[m_faceGroupCount].cb.materialIndex = pFaceGroup->pMaterialHandle->index;
    }

    ID3D12Resource *pVertices = (m_type == RENDER_ITEM_TYPE_CHAR_OBJ) ? m_pDeformedVertexBuffer : m_pVertexBuffer;
    AddBLASGeometry(m_faceGroupCount, pVertices, 0, m_vertexCount, sizeof(BasicVertex), pIndexBuffer, 0,
                    numTriangles * 3, 0, 0);

    m_faceGroupCount++;
    result = TRUE;
lb_return:
    return result;
}

void D3DMeshObject::EndCreateMesh() {}

void D3DMeshObject::EndCreateMesh(ID3D12GraphicsCommandList4 *pCommandList)
{
#ifdef USE_RAYTRACING
    CreateSkinningBufferSRVs();
    CreateRootArgsSRV();
    CreateBottomLevelAS(pCommandList);
#endif
}

void D3DMeshObject::AddBLASGeometry(UINT faceGroupIndex, ID3D12Resource *vertexBuffer, UINT64 vertexOffsetInBytes,
                                    uint32_t vertexCount, UINT vertexSizeInBytes, ID3D12Resource *indexBuffer,
                                    UINT64 indexOffsetInBytes, uint32_t indexCount, ID3D12Resource *transformBuffer,
                                    UINT64 transformOffsetInBytes, bool isOpaque)
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
}

void D3DMeshObject::DeformingVerticesUAV(ID3D12GraphicsCommandList4 *pCommandList, const Matrix *pBoneMats,
                                         UINT numBones)
{
    if (m_type != RENDER_ITEM_TYPE_CHAR_OBJ)
    {
        __debugbreak();
        return;
    }

    ID3D12Device5      *pDevice = m_pRenderer->INL_GetD3DDevice();
    ConstantBufferPool *pSkinnedConstantBufferPool =
        m_pRenderer->GetConstantBufferPool(CONSTANT_BUFFER_TYPE_SKINNED, 0);
    DescriptorPool       *pDescriptorPool = m_pRenderer->INL_GetDescriptorPool(0);
    ID3D12DescriptorHeap *pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();
    UINT descriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // CB Data Binding
    CB_CONTAINER *pSkinnedCB = nullptr;
    pSkinnedCB = pSkinnedConstantBufferPool->Alloc();

    Matrix *pBoneTMs = (Matrix *)pSkinnedCB->pSystemMemAddr;
    for (UINT i = 0; i < numBones; i++)
    {
        Matrix *pBoneTM = pBoneTMs + i;
        memcpy(pBoneTM, &pBoneMats[i].Transpose(), sizeof(Matrix));
    }

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = {};
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = {};

    pDescriptorPool->Alloc(&cpuHandle, &gpuHandle, 3);

    CD3DX12_CPU_DESCRIPTOR_HANDLE dest(cpuHandle);
    pDevice->CopyDescriptorsSimple(SKINNING_DESCRIPTOR_INDEX_COUNT, dest, m_skinningDescriptors.cpuHandle,
                                   D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    dest.Offset(m_descriptorSize, SKINNING_DESCRIPTOR_INDEX_COUNT);
    pDevice->CopyDescriptorsSimple(1, dest, pSkinnedCB->CBVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

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

BOOL D3DMeshObject::CreateBottomLevelAS(ID3D12GraphicsCommandList4 *pCommandList)
{
    BOOL result = FALSE;

    ID3D12Device5  *pDevice = m_pRenderer->INL_GetD3DDevice();
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

BOOL D3DMeshObject::BuildBottomLevelAS(
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

void D3DMeshObject::CreateSkinningBufferSRVs()
{
    if (m_type == RENDER_ITEM_TYPE_MESH_OBJ)
        return;

    ID3D12Device         *pD3DDevice = m_pRenderer->INL_GetD3DDevice();
    D3D12ResourceManager *resourceManager = m_pRenderer->INL_GetResourceManager();

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
    pD3DDevice->CreateShaderResourceView(m_pVertexBuffer, &srvDesc, cpuHandle);
    cpuHandle.Offset(m_descriptorSize);

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.Buffer.CounterOffsetInBytes = 0;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
    uavDesc.Buffer.NumElements = m_vertexCount;
    uavDesc.Buffer.StructureByteStride = sizeof(BasicVertex);
    pD3DDevice->CreateUnorderedAccessView(m_pDeformedVertexBuffer, nullptr, &uavDesc, cpuHandle);
}

void D3DMeshObject::CreateRootArgsSRV()
{
    ID3D12Device                 *pD3DDevice = m_pRenderer->INL_GetD3DDevice();
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
    pD3DDevice->CreateShaderResourceView(pResource, &srvDesc, cpuHandle);
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
        pD3DDevice->CreateShaderResourceView(pFace->pIndexBuffer, &srvDesc, cpuHandle);
        cpuHandle.Offset(m_descriptorSize);

        D3D12_CPU_DESCRIPTOR_HANDLE diffuseTexHandle = !pFace->pAlbedoTexHandle
                                                           ? m_pRenderer->GetDefaultTex()->srv.cpuHandle
                                                           : pFace->pAlbedoTexHandle->srv.cpuHandle;
        pD3DDevice->CopyDescriptorsSimple(1, cpuHandle, diffuseTexHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        cpuHandle.Offset(m_descriptorSize);
    }
}

void D3DMeshObject::CleanupMesh()
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
            if (m_pFaceGroups[i].pMaterialHandle)
            {
                m_pRenderer->DeleteMaterialHandle(m_pFaceGroups[i].pMaterialHandle);
                m_pFaceGroups[i].pMaterialHandle = nullptr;
            }
            if (m_pFaceGroups[i].pAlbedoTexHandle)
            {
                m_pRenderer->DeleteTexture(m_pFaceGroups[i].pAlbedoTexHandle);
                m_pFaceGroups[i].pAlbedoTexHandle = nullptr;
            }
            if (m_pFaceGroups[i].pNormalTexHandle)
            {
                m_pRenderer->DeleteTexture(m_pFaceGroups[i].pNormalTexHandle);
                m_pFaceGroups[i].pNormalTexHandle = nullptr;
            }
            if (m_pFaceGroups[i].pAOTexHandle)
            {
                m_pRenderer->DeleteTexture(m_pFaceGroups[i].pAOTexHandle);
                m_pFaceGroups[i].pAOTexHandle = nullptr;
            }
            if (m_pFaceGroups[i].pMetallicRoughnessTexHandle)
            {
                m_pRenderer->DeleteTexture(m_pFaceGroups[i].pMetallicRoughnessTexHandle);
                m_pFaceGroups[i].pMetallicRoughnessTexHandle = nullptr;
            }
            if (m_pFaceGroups[i].pEmissiveTexHandle)
            {
                m_pRenderer->DeleteTexture(m_pFaceGroups[i].pEmissiveTexHandle);
                m_pFaceGroups[i].pEmissiveTexHandle = nullptr;
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
        m_pRenderer->INL_GetResourceManager()->DeallocDescriptorTable(&m_skinningDescriptors);
        m_skinningDescriptors = {};
    }
}

void D3DMeshObject::Cleanup() { CleanupMesh(); }

D3DMeshObject::~D3DMeshObject() { Cleanup(); }

HRESULT __stdcall D3DMeshObject::QueryInterface(REFIID riid, void **ppvObject) { return E_NOTIMPL; }

ULONG __stdcall D3DMeshObject::AddRef(void) { return ++m_refCount; }

ULONG __stdcall D3DMeshObject::Release(void)
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
