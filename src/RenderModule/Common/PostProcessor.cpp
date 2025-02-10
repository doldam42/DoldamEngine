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

#include "PostProcessor.h"

BOOL PostProcessor::InitMesh()
{
    BOOL result = FALSE;

    ID3D12Device5        *pD3DDeivce = m_pRenderer->GetD3DDevice();
    D3D12ResourceManager *pResourceManager = m_pRenderer->GetResourceManager();

    UINT srvDescriptorSize = m_pRenderer->GetSRVDescriptorSize();

    // Create the vertex buffer.
    // Define the geometry for a triangle.
    SimpleVertex Vertices[] = {
        {{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
    };

    uint32_t Indices[] = {0, 1, 2, 0, 2, 3};

    const UINT VertexBufferSize = sizeof(Vertices);

    if (FAILED(pResourceManager->CreateVertexBuffer(&m_pVertexBuffer, &m_vertexBufferView, sizeof(SimpleVertex),
                                                    (UINT)_countof(Vertices), Vertices)))
    {
        __debugbreak();
        goto lb_return;
    }
    if (FAILED(
            pResourceManager->CreateIndexBuffer(&m_pIndexBuffer, &m_indexBufferView, (UINT)_countof(Indices), Indices)))
    {
        __debugbreak();
        goto lb_return;
    }
    result = TRUE;

lb_return:
    return result;
}

void PostProcessor::Cleanup() 
{ 
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

BOOL PostProcessor::Initialize(D3D12Renderer *pRenderer)
{
    BOOL result = FALSE;

    m_pRenderer = pRenderer;

    InitMesh();

    return TRUE;
}

void PostProcessor::Draw(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList, D3D12_VIEWPORT *pViewport,
                         D3D12_RECT *pScissorRect, D3D12_CPU_DESCRIPTOR_HANDLE src,
                         D3D12_CPU_DESCRIPTOR_HANDLE renderTarget)
{
    ID3D12Device5        *pD3DDevice = m_pRenderer->GetD3DDevice();
    UINT                  srvDescriptorSize = m_pRenderer->GetSRVDescriptorSize();
    DescriptorPool       *pDescriptorPool = m_pRenderer->GetDescriptorPool(threadIndex);
    ID3D12DescriptorHeap *pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;

    pDescriptorPool->Alloc(&cpuHandle, &gpuHandle, 1);
    pD3DDevice->CopyDescriptorsSimple(1, cpuHandle, src, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    pCommandList->RSSetViewports(1, pViewport);
    pCommandList->RSSetScissorRects(1, pScissorRect);
    pCommandList->OMSetRenderTargets(1, &renderTarget, FALSE, nullptr);
    pCommandList->SetGraphicsRootSignature(Graphics::presentRS);
    pCommandList->SetPipelineState(Graphics::presentPSO);

    pCommandList->SetDescriptorHeaps(1, &pDescriptorHeap);
    pCommandList->SetGraphicsRootDescriptorTable(0, gpuHandle);

    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    pCommandList->IASetIndexBuffer(&m_indexBufferView);
    pCommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

PostProcessor::~PostProcessor() { Cleanup(); }
