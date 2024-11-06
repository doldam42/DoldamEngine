#pragma once

const UINT MESH_OBJECT_COUNT_FOR_SHADOW = 2048;

class D3D12Renderer;
class ShadowManager
{
    D3D12Renderer *m_pRenderer = nullptr;
    UINT           m_shadowWidth;
    D3D12_VIEWPORT m_shadowViewports;
    D3D12_RECT     m_shadowScissorRects;
    DXGI_FORMAT    m_shadowMapFormat;

    CB_CONTAINER   *m_pShadowGlobalCB = nullptr;
    GlobalConstants m_shadowGlobalConsts = {};

    Matrix m_shadowProj;
    Matrix m_shadowView;

    ID3D12Resource   *m_pShadowDepthStencils = nullptr;
    DESCRIPTOR_HANDLE m_shadowMapRTV;
    DESCRIPTOR_HANDLE m_shadowMapSRV;
    DESCRIPTOR_HANDLE m_shadowMapDSV;

    // Rendering 해야할 전체 Object의 AABB를 구하기 위해 사용
    Vector3     m_sceneMinCorner = Vector3(FLT_MAX);
    Vector3     m_sceneMaxCorner = Vector3(FLT_MIN);

    // Render Queue for draw shadow
    RenderQueue *m_pRenderQueue = nullptr;

    // Debuggin용 Texture
    TEXTURE_HANDLE *m_pShadowMapTexture = nullptr;

  private:
    void Cleanup();

    void UpdateGlobalConstants();

    void CreateAABBPoints(Vector3 *pOutAABBPoints, const Vector3 &center, const Vector3 &extends);
    void ComputeNearAndFar(FLOAT &fNearPlane, FLOAT &fFarPlane, DirectX::FXMVECTOR vLightCameraOrthographicMin,
                           DirectX::FXMVECTOR vLightCameraOrthographicMax, Vector3 *pvPointsInCameraView);

  public:
    // Shadow width equal height
    BOOL Initialize(D3D12Renderer *pRnd, UINT shadowWidth);

    BOOL Add(const RENDER_ITEM *pItem);

    BOOL Update(const Matrix &lightCameraView, const Matrix &viewerCameraView, const Matrix &viewerCameraProjection,
                float nearZ, float farZ);

    TEXTURE_HANDLE *GetShadowMapTexture() const { return m_pShadowMapTexture; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetShadowMapDescriptorHandle() const { return m_shadowMapSRV.cpuHandle; }

    void Render(ID3D12CommandQueue *pCommandQueue);

    void Reset();

    const Matrix &GetShadowViewMatrix() const { return m_shadowView; }
    const Matrix &GetShadowProjMatrix() const { return m_shadowProj; }

    ShadowManager() = default;
    ~ShadowManager();
};
