#pragma once

#include "D3D12Renderer.h"

class ShadowManager
{
    D3D12Renderer *m_pRenderer = nullptr;
    UINT           m_shadowWidth;
    D3D12_VIEWPORT m_shadowViewports;
    D3D12_RECT     m_shadowScissorRects;
    DXGI_FORMAT    m_shadowMapFormat;

    Matrix m_shadowProj;
    Matrix m_shadowView;

    ID3D12Resource   *m_pShadowDepthStencils = nullptr;
    DESCRIPTOR_HANDLE m_shadowMapRTV;
    DESCRIPTOR_HANDLE m_shadowMapSRV;
    DESCRIPTOR_HANDLE m_shadowMapDSV;

    // Scene¿ë
    BoundingBox m_sceneBox;
    
    // Render Queue for draw shadow
    RenderQueue *m_pRenderQueue = nullptr;

    // Debuggin¿ë Texture
    TEXTURE_HANDLE *m_pShadowMapTexture = nullptr;

  private:
    void Cleanup();
    void CreateAABBPoints(Vector3 *pOutAABBPoints, const Vector3 &center, const Vector3 &extends);
    void ComputeNearAndFar(FLOAT &fNearPlane, FLOAT &fFarPlane, DirectX::FXMVECTOR vLightCameraOrthographicMin,
                           DirectX::FXMVECTOR vLightCameraOrthographicMax, Vector3 *pvPointsInCameraView);

  public:
    // Shadow width equal height
    BOOL Initialize(D3D12Renderer *pRnd, UINT shadowWidth);

    BOOL Add(const RENDER_ITEM *pItem);

    BOOL Update(const Matrix &viewerCameraView, const Matrix &viewerCameraProjection, const Matrix &lightCameraView,
                const Matrix &lightCameraProjection, float nearZ, float farZ);

    TEXTURE_HANDLE *GetShadowMapTexture() const { return m_pShadowMapTexture; }

    void Render(ID3D12CommandQueue *pCommandQueue);

    void Reset();

    const Matrix &GetShadowViewMatrix() const { return m_shadowView; }
    const Matrix &GetShadowProjMatrix() const { return m_shadowProj; }

    ShadowManager() = default;
    ~ShadowManager();
};
