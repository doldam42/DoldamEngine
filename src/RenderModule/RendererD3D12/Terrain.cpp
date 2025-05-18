#include "pch.h"

#include <stb/stb_image.h>

#include "ConstantBufferPool.h"
#include "D3D12Renderer.h"
#include "D3D12ResourceManager.h"
#include "DescriptorPool.h"
#include "MaterialManager.h"
#include "GraphicsCommon.h"

#include "Terrain.h"

BOOL Terrain::InitMesh(const WCHAR *heightFilename, const int numSlice, const int numStack)
{
    BOOL                  result = FALSE;
    D3D12ResourceManager *pResourceManager = m_pRenderer->GetResourceManager();

    uint8_t *pVertices = new uint8_t[(numStack + 1) * (numSlice + 1)];
    UINT    *pIndices = new UINT[numStack * numSlice * 4];

    ZeroMemory(pVertices, sizeof(uint8_t) * (numStack + 1) * (numSlice + 1));
    ZeroMemory(pIndices, sizeof(UINT) * numStack * numSlice * 4);

    UINT numVertices = 0;
    UINT numIndices = 0;

    char filename[MAX_PATH] = {'\0'};
    ws2s(heightFilename, filename);

    int      width, height, channels;
    uint8_t *pImage = stbi_load(filename, &width, &height, &channels, 0);
    assert(pImage != NULL);
    //assert(channels == 1);

    const float dx = float(width - 1) / numSlice;
    const float dy = float(height - 1) / numStack;
    for (int j = 0; j < numStack + 1; j++)
    {
        int y = j * dy;
        for (int i = 0; i < numSlice + 1; i++)
        {
            int x = i * dx;
            uint8_t *pSrc = pImage + (x + y * width) * channels;
            pVertices[numVertices++] = *pSrc;
        }
    }
    /*for (int y = 0; y < height; y += dy)
    {
        for (int x = 0; x < width; x += dx)
        {
            uint8_t *pSrc = pImage + (x + y * width) * channels;
            pVertices[numVertices++] = *pSrc;
        }
    }*/

    // | (numSlice + 1) * j + i | (numSlice + 1) * j + i + 1 |
    // | (numSlice + 1) * (j + 1) + i | (numSlice + 1) * (j + 1) + i + 1 |
    for (int j = 0; j < numStack; j++)
    {
        for (int i = 0; i < numSlice; i++)
        {
            pIndices[numIndices++] = (numSlice + 1) * j + i;
            pIndices[numIndices++] = (numSlice + 1) * j + i + 1;
            pIndices[numIndices++] = (numSlice + 1) * (j + 1) + i;
            pIndices[numIndices++] = (numSlice + 1) * (j + 1) + i + 1;
        }
    }

    if (FAILED(pResourceManager->CreateVertexBuffer(&m_pVertexBuffer, &m_vertexBufferView, sizeof(uint8_t), numVertices,
                                                    pVertices)))
    {
        __debugbreak();
        goto lb_return;
    }

    if (FAILED(pResourceManager->CreateIndexBuffer(&m_pIndexBuffer, &m_indexBufferView, numIndices, pIndices)))
    {
        __debugbreak();
        goto lb_return;
    }

    m_vertexCount = numVertices;
    m_indexCount = numIndices;

    m_numSlices = numSlice;
    m_numStacks = numStack;

    result = TRUE;
lb_return:
    if (pIndices)
    {
        delete[] pIndices;
        pIndices = nullptr;
    }
    if (pVertices)
    {
        delete[] pVertices;
        pVertices = nullptr;
    }
    if (pImage != NULL)
    {
        delete[] pImage;
        pImage = nullptr;
    }
    return result;
}

void Terrain::Cleanup()
{
    if (m_pMaterialHandle)
    {
        m_pMaterialHandle->Release();
        m_pMaterialHandle = nullptr;
    }
    if (m_pVertexBuffer)
    {
        m_pVertexBuffer->Release();
        m_pVertexBuffer = nullptr;
    }
    if (m_pIndexBuffer)
    {
        m_pIndexBuffer->Release();
        m_pIndexBuffer = nullptr;
    }
}

BOOL Terrain::Initialize(D3D12Renderer *pRenderer, const Vector3 *pScale, const Material *pMaterial,
                         const int numSlice, const int numStack)
{
    m_pRenderer = pRenderer;
    m_pD3DDevice = pRenderer->GetD3DDevice();

    WCHAR heightFilename[MAX_PATH] = {L'\0'};
    wcscpy_s(heightFilename, pMaterial->basePath);
    wcscat_s(heightFilename, pMaterial->heightTextureName);
    InitMesh(heightFilename, numSlice, numStack);

    MaterialManager *pMatManager = m_pRenderer->GetMaterialManager();
    m_pMaterialHandle = pMatManager->CreateMaterial(pMaterial, MATERIAL_TYPE_DEFAULT);

    m_scale = *pScale;
    m_descriptorSize = pRenderer->GetSRVDescriptorSize();

    return TRUE;
}

void Terrain::Draw(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList, D3D12_GPU_DESCRIPTOR_HANDLE globalCBV,
                   DRAW_PASS_TYPE passType, const Vector3 *pScale, FILL_MODE fillMode)
{
    // TODO: ADD Shadow
    if (passType == DRAW_PASS_TYPE_SHADOW)
        return;
    ID3D12RootSignature *pRS = Graphics::GetRS(RENDER_ITEM_TYPE_TERRAIN, passType);
    ID3D12PipelineState *pPSO = Graphics::GetPSO(RENDER_ITEM_TYPE_TERRAIN, passType, fillMode);

    DescriptorPool       *pDescriptorPool = m_pRenderer->GetDescriptorPool(threadIndex);
    ID3D12DescriptorHeap *pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();

    // | Terrain Constants(b1) | Material(b5) | Textures(t0~t6) |
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle = {};
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle = {};
    pDescriptorPool->Alloc(&cpuHandle, &gpuHandle, DESCRIPTOR_COUNT_PER_DRAW);

    // Update Terrain Constant Buffer
    CB_CONTAINER       *pTerrainCB;
    ConstantBufferPool *pTerrainConstantBufferPool =
        m_pRenderer->GetConstantBufferPool(CONSTANT_BUFFER_TYPE_TERRAIN, threadIndex);
    pTerrainCB = pTerrainConstantBufferPool->Alloc();

    TerrainConstants *pTerrainConsts = (TerrainConstants *)pTerrainCB->pSystemMemAddr;
    pTerrainConsts->numSlice = m_numSlices;
    pTerrainConsts->numStack = m_numStacks;
    pTerrainConsts->scaleX = pScale->x;
    pTerrainConsts->scaleY = pScale->y;
    pTerrainConsts->scaleZ = pScale->z;
    pTerrainConsts->tessFactor = 16.0f;

    m_pD3DDevice->CopyDescriptorsSimple(1, cpuHandle, pTerrainCB->CBVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    cpuHandle.Offset(m_descriptorSize);

    // Update Geometry Constant buffer
    CB_CONTAINER *pGeomCB;
    ConstantBufferPool *pGeometryConstantBufferPool =
        m_pRenderer->GetConstantBufferPool(CONSTANT_BUFFER_TYPE_GEOMETRY, threadIndex);
    pGeomCB = pGeometryConstantBufferPool->Alloc();
    FaceGroupConstants *pGeometry = (FaceGroupConstants *)pGeomCB->pSystemMemAddr;
    pGeometry->materialIndex = m_pMaterialHandle->index;
    pGeometry->useHeightMap = TRUE;
    pGeometry->useMaterial = TRUE;

    m_pD3DDevice->CopyDescriptorsSimple(1, cpuHandle, pGeomCB->CBVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    cpuHandle.Offset(m_descriptorSize);

    m_pMaterialHandle->CopyDescriptors(m_pD3DDevice, cpuHandle, m_descriptorSize);

    // set RootSignature
    pCommandList->SetGraphicsRootSignature(pRS);
    ID3D12DescriptorHeap *ppHeaps[] = {pDescriptorHeap};
    pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    pCommandList->SetPipelineState(pPSO);
    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

    pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    pCommandList->SetGraphicsRootDescriptorTable(0, globalCBV);
    pCommandList->SetGraphicsRootDescriptorTable(1, gpuHandle);
    gpuHandle.Offset(m_descriptorSize);
    pCommandList->SetGraphicsRootDescriptorTable(2, gpuHandle);
    pCommandList->IASetIndexBuffer(&m_indexBufferView);
    pCommandList->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
}

Terrain::~Terrain() { Cleanup(); }

HRESULT __stdcall Terrain::QueryInterface(REFIID riid, void **ppvObject) { return E_NOTIMPL; }

ULONG __stdcall Terrain::AddRef(void) { return ++m_refCount; }

ULONG __stdcall Terrain::Release(void) 
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
