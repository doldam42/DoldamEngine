#include "pch.h"

#include "ConstantBufferPool.h"
#include "D3D12Renderer.h"
#include "D3D12ResourceManager.h"
#include "DescriptorPool.h"
#include "GraphicsCommon.h"

#include "Cubemap.h"

BOOL Cubemap::InitMesh()
{
    BOOL result = FALSE;

    ID3D12Device5        *pD3DDeivce = m_pRenderer->INL_GetD3DDevice();
    UINT                  srvDescriptorSize = m_pRenderer->GetSRVDescriptorSize();
    D3D12ResourceManager *pResourceManager = m_pRenderer->INL_GetResourceManager();

    SimpleVertex Vertices[] = {
        // À­¸é
        {{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f}},
        {{-1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
        {{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
        {{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f}},
        // ¾Æ·§¸é
        {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f}},
        {{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f}},
        {{1.0f, -1.0f, 1.0f}, {1.0f, 1.0f}},
        {{-1.0f, -1.0f, 1.0f}, {0.0f, 1.0f}},
        // ¾Õ¸é
        {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f}},
        {{-1.0f, 1.0f, -1.0f}, {1.0f, 0.0f}},
        {{1.0f, 1.0f, -1.0f}, {1.0f, 1.0f}},
        {{1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},
        // µÞ¸é
        {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f}},
        {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f}},
        {{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
        {{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
        // ¿ÞÂÊ
        {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f}},
        {{-1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
        {{-1.0f, 1.0f, -1.0f}, {1.0f, 1.0f}},
        {{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},
        // ¿À¸¥ÂÊ
        {{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f}},
        {{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f}},
        {{1.0f, 1.0f, -1.0f}, {1.0f, 1.0f}},
        {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
    };

    uint32_t indices[] = {
        0,  1,  2,  0,  2,  3,  // À­¸é
        4,  5,  6,  4,  6,  7,  // ¾Æ·§¸é
        8,  9,  10, 8,  10, 11, // ¾Õ¸é
        12, 13, 14, 12, 14, 15, // µÞ¸é
        16, 17, 18, 16, 18, 19, // ¿ÞÂÊ
        20, 21, 22, 20, 22, 23  // ¿À¸¥ÂÊ
    };

    // scaling
    float scale = 50.0f;
    UINT vertexCount = (UINT)_countof(Vertices);
    for (UINT i = 0; i < vertexCount; i++)
    {
        Vertices[i].position *= scale;
    }

    // Revert indices
    UINT indexCount = (UINT)_countof(indices);
    for (UINT i = 0; i < indexCount / 2; i++)
    {
        UINT temp = indices[i];
        indices[i] = indices[indexCount - 1 - i];
        indices[indexCount - 1 - i] = temp;
    }

    if (FAILED(pResourceManager->CreateVertexBuffer(&m_pVertexBuffer, &m_vertexBufferView, sizeof(SimpleVertex),
                                                    vertexCount, Vertices)))
    {
        __debugbreak();
        goto lb_return;
    }
    if (FAILED(pResourceManager->CreateIndexBuffer(&m_pIndexBuffer, &m_indexBufferView, indexCount, indices)))
    {
        __debugbreak();
        goto lb_return;
    }

    result = TRUE;
lb_return:
    return result;
}

void Cubemap::Cleanup()
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

    if (m_pEnv)
    {
        m_pRenderer->DeleteTexture(m_pEnv);
        m_pEnv = nullptr;
    }
    if (m_pSpecular)
    {
        m_pRenderer->DeleteTexture(m_pSpecular);
        m_pSpecular = nullptr;
    }
    if (m_pIrradiance)
    {
        m_pRenderer->DeleteTexture(m_pIrradiance);
        m_pIrradiance = nullptr;
    }
    if (m_pBrdf)
    {
        m_pRenderer->DeleteTexture(m_pBrdf);
        m_pBrdf = nullptr;
    }
}

void Cubemap::Initialize(D3D12Renderer *pRnd, const WCHAR *envFilename, const WCHAR *specularFilename,
                         const WCHAR *irradianceFilename, const WCHAR *brdfFilename)
{
    m_pEnv = (TEXTURE_HANDLE *)pRnd->CreateCubemapFromFile(envFilename);
    m_pSpecular = (TEXTURE_HANDLE *)pRnd->CreateCubemapFromFile(specularFilename);
    m_pIrradiance = (TEXTURE_HANDLE *)pRnd->CreateCubemapFromFile(irradianceFilename);
    m_pBrdf = (TEXTURE_HANDLE *)pRnd->CreateTextureFromFile(brdfFilename);

    m_pRenderer = pRnd;

    InitMesh();
}

D3D12_CPU_DESCRIPTOR_HANDLE Cubemap::GetEnvSRV()
{
    return m_pEnv->srv.cpuHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE Cubemap::GetSpecularSRV()
{
    return m_pSpecular->srv.cpuHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE Cubemap::GetIrradianceSRV()
{
    return m_pIrradiance->srv.cpuHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE Cubemap::GetBrdfSRV()
{
    return m_pBrdf->srv.cpuHandle;
}

void Cubemap::Draw(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList)
{
    ID3D12Device5        *pD3DDevice = m_pRenderer->INL_GetD3DDevice();
    UINT                  srvDescriptorSize = m_pRenderer->GetSRVDescriptorSize();
    DescriptorPool       *pDescriptorPool = m_pRenderer->INL_GetDescriptorPool(threadIndex);
    ID3D12DescriptorHeap *pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    pDescriptorPool->Alloc(&cpuHandle, &gpuHandle, DESCRIPOTR_INDEX_PER_CUBEMAP_COUNT);

    CD3DX12_CPU_DESCRIPTOR_HANDLE dest(cpuHandle);
    pD3DDevice->CopyDescriptorsSimple(1, dest, m_pEnv->srv.cpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    dest.Offset(srvDescriptorSize);
    pD3DDevice->CopyDescriptorsSimple(1, dest, m_pSpecular->srv.cpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    dest.Offset(srvDescriptorSize);
    pD3DDevice->CopyDescriptorsSimple(1, dest, m_pIrradiance->srv.cpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    dest.Offset(srvDescriptorSize);
    pD3DDevice->CopyDescriptorsSimple(1, dest, m_pBrdf->srv.cpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    pCommandList->SetGraphicsRootSignature(Graphics::GetRS(RENDER_ITEM_TYPE_SKYBOX, DRAW_PASS_TYPE_DEFAULT));
    pCommandList->SetDescriptorHeaps(1, &pDescriptorHeap);
    pCommandList->SetGraphicsRootConstantBufferView(0, m_pRenderer->INL_GetGlobalCB()->pGPUMemAddr);
    pCommandList->SetGraphicsRootDescriptorTable(1, gpuHandle);

    pCommandList->SetPipelineState(Graphics::GetPSO(RENDER_ITEM_TYPE_SKYBOX, DRAW_PASS_TYPE_DEFAULT, FILL_MODE_SOLID));
    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    pCommandList->IASetIndexBuffer(&m_indexBufferView);
    pCommandList->DrawIndexedInstanced(36, 1, 0, 0, 0);
}

Cubemap::~Cubemap()
{
    Cleanup();
}
