#include "pch.h"

#include "ConstantBufferPool.h"
#include "D3D12Renderer.h"
#include "D3D12ResourceManager.h"
#include "DescriptorPool.h"
#include "GraphicsCommon.h"

#include "SpriteObject.h"

ID3D12Resource *SpriteObject::m_pVertexBuffer = nullptr;
D3D12_VERTEX_BUFFER_VIEW SpriteObject::m_vertexBufferView = {};

ID3D12Resource *SpriteObject::m_pIndexBuffer = nullptr;
D3D12_INDEX_BUFFER_VIEW SpriteObject::m_indexBufferView = {};

ULONG SpriteObject::m_initRefCount = 0;

BOOL SpriteObject::InitSharedResources()
{
    if (m_initRefCount)
        goto lb_return;

    InitMesh();
lb_return:
    m_initRefCount++;
    return m_initRefCount;
}

void SpriteObject::CleanupSharedResources()
{
    if (!m_initRefCount)
        return;

    UINT ref_count = --m_initRefCount;
    if (!ref_count)
    {
        if (m_pIndexBuffer)
        {
            m_pIndexBuffer->Release();
            m_pIndexBuffer = nullptr;
        }
        if (m_pVertexBuffer)
        {
            m_pVertexBuffer->Release();
            m_pVertexBuffer = nullptr;
        }
    }
}

BOOL SpriteObject::InitMesh()
{
    BOOL result = FALSE;

    ID3D12Device5        *pD3DDeivce = m_pRenderer->GetD3DDevice();
    UINT                  srvDescriptorSize = m_pRenderer->GetResourceManager()->GetDescriptorSize();
    D3D12ResourceManager *pResourceManager = m_pRenderer->GetResourceManager();

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

void SpriteObject::Cleanup()
{
    if (m_pTexHandle)
    {
        m_pRenderer->DeleteTexture(m_pTexHandle);
        m_pTexHandle = nullptr;
    }
    CleanupSharedResources();
}

BOOL SpriteObject::Initialize(D3D12Renderer *pRenderer)
{
    m_pRenderer = pRenderer;

    BOOL result = InitSharedResources();
    return result;
}

BOOL SpriteObject::Initialize(D3D12Renderer *pRenderer, const WCHAR *texFileName, const RECT *pRect)
{
    m_pRenderer = pRenderer;

    BOOL result = (InitSharedResources() != 0);
    if (result)
    {
        UINT texWidth = 1;
        UINT texHeight = 1;
        m_pTexHandle = (TEXTURE_HANDLE *)m_pRenderer->CreateTextureFromFile(texFileName);
        if (m_pTexHandle)
        {
            D3D12_RESOURCE_DESC desc = m_pTexHandle->pTexture->GetDesc();
            texWidth = desc.Width;
            texHeight = desc.Height;
        }
        if (pRect)
        {
            m_Rect = *pRect;
            m_scale.x = (float)(m_Rect.right - m_Rect.left) / (float)texWidth;
            m_scale.y = (float)(m_Rect.bottom - m_Rect.top) / (float)texHeight;
        }
        else
        {
            if (m_pTexHandle)
            {
                D3D12_RESOURCE_DESC desc = m_pTexHandle->pTexture->GetDesc();
                m_Rect.left = 0;
                m_Rect.top = 0;
                m_Rect.right = desc.Width;
                m_Rect.bottom = desc.Height;
            }
        }
    }

    return result;
}

void SpriteObject::DrawWithTex(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList, const Vector2 *pPos,
                               const Vector2 *pScale,
                               const RECT *pRect, float Z, TEXTURE_HANDLE *pTexHandle)
{
    ID3D12Device5        *pD3DDeivce = m_pRenderer->GetD3DDevice();
    UINT                  srvDescriptorSize = m_pRenderer->GetResourceManager()->GetDescriptorSize();
    DescriptorPool       *pDescriptorPool = m_pRenderer->GetDescriptorPool(threadIndex);
    ID3D12DescriptorHeap *pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();
    ConstantBufferPool   *pConstantBufferPool = m_pRenderer->GetConstantBufferPool(CONSTANT_BUFFER_TYPE_SPRITE, threadIndex);

    UINT TexWidth = 0;
    UINT TexHeight = 0;

    D3D12_CPU_DESCRIPTOR_HANDLE srv = {};
    if (pTexHandle)
    {
        D3D12_RESOURCE_DESC desc = pTexHandle->pTexture->GetDesc();
        TexWidth = desc.Width;
        TexHeight = desc.Height;
        srv = pTexHandle->srv.cpuHandle;
    }

    RECT rect;
    if (!pRect)
    {
        rect.left = 0;
        rect.top = 0;
        rect.right = TexWidth;
        rect.bottom = TexHeight;
        pRect = &rect;
    }

    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescriptorTable = {};
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescriptorTable = {};

    if (!pDescriptorPool->Alloc(&cpuDescriptorTable, &gpuDescriptorTable, DESCRIPTOR_COUNT_PER_DRAW))
    {
        __debugbreak();
    }

    // ������ draw()�� ���� �������� constant buffer(���������δ� ���� resource�� �ٸ� ����)�� ����Ѵ�.
    CB_CONTAINER *pCB = pConstantBufferPool->Alloc();
    if (!pCB)
    {
        __debugbreak();
    }

    SpriteConstants *pConstantBufferSprite = (SpriteConstants *)pCB->pSystemMemAddr;

    // constant buffer�� ������ ����
    pConstantBufferSprite->screenRes.x = (float)m_pRenderer->INL_GetScreenWidth();
    pConstantBufferSprite->screenRes.y = (float)m_pRenderer->INL_GetScreenHeight();
    pConstantBufferSprite->pos = *pPos;
    pConstantBufferSprite->scale = *pScale;
    pConstantBufferSprite->texSize.x = (float)TexWidth;
    pConstantBufferSprite->texSize.y = (float)TexHeight;
    pConstantBufferSprite->texSamplePos.x = (float)pRect->left;
    pConstantBufferSprite->texSamplePos.y = (float)pRect->top;
    pConstantBufferSprite->texSampleSize.x = (float)(pRect->right - pRect->left);
    pConstantBufferSprite->texSampleSize.y = (float)(pRect->bottom - pRect->top);
    pConstantBufferSprite->z = Z;
    pConstantBufferSprite->alpha = 1.0f;

    // set RootSignature
    pCommandList->SetGraphicsRootSignature(Graphics::GetRS(RENDER_ITEM_TYPE_SPRITE, DRAW_PASS_TYPE_DEFAULT));
    pCommandList->SetDescriptorHeaps(1, &pDescriptorHeap);

    // Descriptor Table ����
    // �̹��� ����� constant buffer�� descriptor�� ��������(shader visible) descriptor table�� ī��

    CD3DX12_CPU_DESCRIPTOR_HANDLE cbvDest(cpuDescriptorTable, SPRITE_DESCRIPTOR_INDEX_CBV, srvDescriptorSize);
    pD3DDeivce->CopyDescriptorsSimple(1, cbvDest, pCB->CBVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    if (srv.ptr)
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE srvDest(cpuDescriptorTable, SPRITE_DESCRIPTOR_INDEX_TEX, srvDescriptorSize);
        pD3DDeivce->CopyDescriptorsSimple(1, srvDest, srv, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    pCommandList->SetGraphicsRootDescriptorTable(0, gpuDescriptorTable);

    pCommandList->SetPipelineState(Graphics::GetPSO(RENDER_ITEM_TYPE_SPRITE, DRAW_PASS_TYPE_DEFAULT, FILL_MODE_SOLID));
    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    pCommandList->IASetIndexBuffer(&m_indexBufferView);
    pCommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void SpriteObject::Draw(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList, const Vector2 *pPos,
                        const Vector2 *pScale, float Z)
{
    Vector2 scale = {m_scale.x * pScale->x, m_scale.y * pScale->y};
    DrawWithTex(threadIndex, pCommandList, pPos, &scale, &m_Rect, Z, m_pTexHandle);
}

SpriteObject::~SpriteObject()
{
    Cleanup();
}

HRESULT __stdcall SpriteObject::QueryInterface(REFIID riid, void **ppvObject)
{
    return E_NOTIMPL;
}

ULONG __stdcall SpriteObject::AddRef(void)
{
    return ++m_refCount;
}

ULONG __stdcall SpriteObject::Release(void)
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
