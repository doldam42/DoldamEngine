#include "pch.h"

#include "D3D12ResourceManager.h"
#include "TextureManager.h"

#include "CascadedShadowsManager.h"

void CascadedShadowsManager::Cleanup()
{
    D3D12ResourceManager *pResourceManager = m_pRenderer->INL_GetResourceManager();

    pResourceManager->DeallocDescriptorTable(&m_CascadedShadowMapSRV);
    pResourceManager->DeallocRTVDescriptorTable(&m_CascadedShadowMapRTV);
    pResourceManager->DeallocDSVDescriptorTable(&m_CascadedShadowMapDSV);
}

BOOL CascadedShadowsManager::Initialize(D3D12Renderer *pRnd, Camera *pViewerCamera, Camera *pLightCamera,
                                        UINT shadowWidth, UINT cascadedLevel)
{
    BOOL result = FALSE;

    ID3D12Device         *pD3DDevice = pRnd->INL_GetD3DDevice();
    D3D12ResourceManager *pResourceManager = pRnd->INL_GetResourceManager();
    TextureManager       *pTextureManager = pRnd->INL_GetTextureManager();

    m_shadowWidth = shadowWidth;
    m_cascadedLevels = cascadedLevel;

    // Init
    m_shadowScissorRects.left = 0;
    m_shadowScissorRects.top = 0;
    m_shadowScissorRects.right = shadowWidth;
    m_shadowScissorRects.bottom = shadowWidth;

    for (int cascadeIndex = 0; cascadeIndex < cascadedLevel; cascadeIndex++)
    {
        m_shadowViewports[cascadeIndex].TopLeftX = float(shadowWidth * cascadeIndex);
        m_shadowViewports[cascadeIndex].TopLeftY = 0;
        m_shadowViewports[cascadeIndex].Width = float(shadowWidth);
        m_shadowViewports[cascadeIndex].Height = float(shadowWidth);
        m_shadowViewports[cascadeIndex].MinDepth = 0.0f;
        m_shadowViewports[cascadeIndex].MaxDepth = 1.0f;
    }

    pResourceManager->AllocDSVDescriptorTable(&m_CascadedShadowMapDSV);
    pResourceManager->AllocRTVDescriptorTable(&m_CascadedShadowMapRTV);
    pResourceManager->AllocDescriptorTable(&m_CascadedShadowMapSRV, 1);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

    D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
    depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
    depthOptimizedClearValue.DepthStencil.Stencil = 0;

    ID3D12Resource *pDepthStencil = nullptr;

    if (FAILED(pD3DDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32_TYPELESS, shadowWidth * cascadedLevel, shadowWidth, 1, 1, 1,
                                          0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
            D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, &depthOptimizedClearValue, IID_PPV_ARGS(&pDepthStencil))))
    {
        __debugbreak();
    }
    pDepthStencil->SetName(L"Cascaded Shadow Map DepthStencil");

    pD3DDevice->CreateShaderResourceView(pDepthStencil, &srvDesc, m_CascadedShadowMapSRV.cpuHandle);
    pD3DDevice->CreateDepthStencilView(pDepthStencil, &dsvDesc, m_CascadedShadowMapDSV.cpuHandle);

    m_pCascadedShadowMapTexture =
        pTextureManager->CreateRenderableTexture(shadowWidth * cascadedLevel, shadowWidth, DXGI_FORMAT_R8G8B8A8_UNORM);
    pD3DDevice->CreateRenderTargetView(m_pCascadedShadowMapTexture->pTexture, nullptr,
                                       m_CascadedShadowMapRTV.cpuHandle);

    m_pCascadedShadowDepthStencils = pDepthStencil;
    m_pLightCamera = pLightCamera;
    m_pViewerCamera = pViewerCamera;

    result = TRUE;
lb_return:

    m_pRenderer = pRnd;

    return result;
}

CascadedShadowsManager::~CascadedShadowsManager() {}

BOOL CascadedShadowsManager::Update()
{
    const Matrix &viewerCameraProjection = m_pViewerCamera->GetProjMatrix();
    const Matrix &viewerCameraView = m_pViewerCamera->GetViewMatrix();

    Matrix viewerCameraViewInverse = viewerCameraView.Invert();

    const Matrix &lightCameraProjection = m_pLightCamera->GetProjMatrix();
    const Matrix &lightCameraView = m_pLightCamera->GetViewMatrix();

    Vector4 sceneCenter = (m_sceneAABBMin + m_sceneAABBMax) * 0.5f;
    Vector4 sceneExtents = (m_sceneAABBMax - m_sceneAABBMin) * 0.5f;

    Vector4 sceneAABBPointsLightSpace[MAX_CASADEDS];
    return FALSE;
}
