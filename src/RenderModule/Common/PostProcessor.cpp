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

BOOL PostProcessor::Initialize(D3D12Renderer *pRenderer)
{
    BOOL result = FALSE;

    m_pRenderer = pRenderer;

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
    pCommandList->IASetVertexBuffers(0, 1, &m_pRenderer->GetFullScreenQuadVertexBufferView());
    pCommandList->DrawInstanced(6, 1, 0, 0);
}

PostProcessor::~PostProcessor() { }
