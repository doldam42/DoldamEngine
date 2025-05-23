#include "pch.h"

#include <d3d12.h>
#include <d3dx12.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>

#include "ConstantBuffers.h"

#include "ConstantBufferPool.h"
#include "D3D12Renderer.h"
#include "D3D12ResourceManager.h"
#include "DescriptorPool.h"
#include "MaterialManager.h"

#include "D3DMeshObject.h"


BOOL D3DMeshObject::Initialize(D3D12Renderer *pRenderer, RENDER_ITEM_TYPE type)
{

    ID3D12Device *pDevice = pRenderer->GetD3DDevice();
    m_descriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    m_pRenderer = pRenderer;
    m_pD3DDevice = pDevice;
    m_type = type;

    return TRUE;
}

void D3DMeshObject::Draw(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList, const Matrix *pWorldMat,
                         IRenderMaterial **ppMaterials, UINT numMaterials, ID3D12RootSignature *pRS,
                         ID3D12PipelineState *pPSO, D3D12_GPU_DESCRIPTOR_HANDLE globalCBV, const Matrix *pBoneMats,
                         UINT numBones, DRAW_PASS_TYPE passType, UINT numInstance)
{   
    DescriptorPool       *pDescriptorPool = m_pRenderer->GetDescriptorPool(threadIndex);
    ID3D12DescriptorHeap *pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = {};
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = {};
    pDescriptorPool->Alloc(&cpuHandle, &gpuHandle, m_descriptorCountPerDraw);

    UpdateDescriptorTablePerObj(cpuHandle, threadIndex, pWorldMat, numInstance, pBoneMats, numBones);
    
    if (passType != DRAW_PASS_TYPE_SHADOW)
    {
        UpdateDescriptorTablePerFaceGroup(cpuHandle, threadIndex, ppMaterials, numMaterials);
    }

    // set RootSignature
    pCommandList->SetGraphicsRootSignature(pRS);
    ID3D12DescriptorHeap *ppHeaps[] = {pDescriptorHeap};
    pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    pCommandList->SetPipelineState(pPSO);
    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    CD3DX12_GPU_DESCRIPTOR_HANDLE _gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(gpuHandle);
    pCommandList->SetGraphicsRootDescriptorTable(0, globalCBV);
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
        if (passType != DRAW_PASS_TYPE_SHADOW)
        {
            pCommandList->SetGraphicsRootDescriptorTable(2, _gpuHandle);
            _gpuHandle.Offset(m_descriptorSize, DESCRIPTOR_INDEX_PER_FACE_GROUP_COUNT);
        }

        INDEXED_FACE_GROUP *pFaceGroup = m_pFaceGroups + i;
        pCommandList->IASetIndexBuffer(&pFaceGroup->IndexBufferView);
        pCommandList->DrawIndexedInstanced(pFaceGroup->numTriangles * 3, numInstance, 0, 0, 0);
    }
}

void D3DMeshObject::RenderNormal(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList, const Matrix *pWorldMat,
                                 const Matrix *pBoneMats, UINT numBones, FILL_MODE fillMode, UINT numInstance)
{
    DescriptorPool       *pDescriptorPool = m_pRenderer->GetDescriptorPool(threadIndex);
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
void D3DMeshObject::UpdateDescriptorTablePerFaceGroup(D3D12_CPU_DESCRIPTOR_HANDLE descriptorTable, UINT threadIndex,
                                                      IRenderMaterial **ppMaterials, UINT numMaterial)
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

BOOL D3DMeshObject::BeginCreateMesh(const void *pVertices, UINT numVertices, const Joint *pJoint, UINT numJoint,
                                    UINT numFaceGroup)
{
    BOOL                  result = FALSE;
    ID3D12Device5        *pD3DDeivce = m_pRenderer->GetD3DDevice();
    D3D12ResourceManager *resourceManager = m_pRenderer->GetResourceManager();

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

    m_descriptorCountPerDraw =
        (m_type == RENDER_ITEM_TYPE_MESH_OBJ) ? DESCRIPTOR_COUNT_PER_STATIC_OBJ : DESCRIPTOR_COUNT_PER_DYNAMIC_OBJ;
    m_descriptorCountPerDraw += numFaceGroup * DESCRIPTOR_INDEX_PER_FACE_GROUP_COUNT;

    result = TRUE;
lb_return:
    return result;
}

BOOL D3DMeshObject::InsertFaceGroup(const UINT *pIndices, UINT numTriangles)
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

    m_faceGroupCount++;
    result = TRUE;
lb_return:
    return result;
}

void D3DMeshObject::EndCreateMesh() {}

void D3DMeshObject::CleanupMesh()
{
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
