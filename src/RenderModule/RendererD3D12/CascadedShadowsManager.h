#pragma once

// reference: DirectX SDK Sample - CascadedShadowMaps11

#include "D3D12Renderer.h"
#include "D3DMeshObject.h"

const UINT MAX_CASADEDS = 8;
const UINT MESH_OBJECT_COUNT_FOR_SHADOW = 2048;

enum FIT_PROJETION_TO
{
    FIT_PROJETION_TO_CASCADES,
    FIT_PROJETION_TO_SCENE
};

enum FIT_NEARFAR
{
    FIT_NEARFAR_PANCAKING,
    FIT_NEARFAR_ZERO_ONE,
    FIT_NEARFAR_AABB,
    FIT_NEARFAR_SCENE_AABB
};

enum CASCADE_SELECTION
{
    CASCADE_SELECTION_MAP,
    CASCADE_SELECTION_INTERVAL
};

enum CAMERA_SELECTION
{
    EYE_CAMERA,
    LIGHT_CAMERA,
    ORTHO_CAMERA1,
    ORTHO_CAMERA2,
    ORTHO_CAMERA3,
    ORTHO_CAMERA4,
    ORTHO_CAMERA5,
    ORTHO_CAMERA6,
    ORTHO_CAMERA7,
    ORTHO_CAMERA8
};

class CascadedShadowsManager
{
  private:
    D3D12Renderer *m_pRenderer = nullptr;

    UINT           m_cascadedLevels;
    UINT           m_shadowWidth;
    D3D12_VIEWPORT m_shadowViewports[MAX_CASADEDS] = {};
    D3D12_RECT     m_shadowScissorRects;
    DXGI_FORMAT    m_shadowMapFormat;

    Matrix m_shadowProj[MAX_CASADEDS];
    Matrix m_shadowView;

    ID3D12Resource   *m_pCascadedShadowDepthStencils = nullptr;
    DESCRIPTOR_HANDLE m_CascadedShadowMapRTV;
    DESCRIPTOR_HANDLE m_CascadedShadowMapSRV;
    DESCRIPTOR_HANDLE m_CascadedShadowMapDSV;

    // MeshObjects for draw shadow
    RenderQueue *m_pRenderQueue = nullptr;
    
    // Debuggin¿ë Texture
    TEXTURE_HANDLE *m_pCascadedShadowMapTexture = nullptr;

  public:
    // Config
    BOOL              m_IsMoveLightTexelSize = FALSE;
    FIT_PROJETION_TO  m_selectedCascadesFit = FIT_PROJETION_TO_SCENE;
    FIT_NEARFAR       m_selectedNearFarFit = FIT_NEARFAR_SCENE_AABB;
    CASCADE_SELECTION m_selectedCascadedSelection = CASCADE_SELECTION_MAP;

    int   m_cascadePartitionsMax = 100;
    float m_cascadePartitionsFrustum[MAX_CASADEDS];
    int   m_cascadePartitionsZeroToOne[MAX_CASADEDS] = {5,   15,  60,  100,
                                                        100, 100, 100, 100}; // Values are  between near and far
    int   m_PCFBlurSize = 3;
    float m_PCFOffset = 0.002;
    int   m_derivativeBasedOffset = 0;
    int   m_blurBetweenCascades = 0;
    float m_blurBetweenCascadesAmount = 0.005;

  private:
    void Cleanup();
    void CreateAABBPoints(Vector3 *pAABBPoints, const Vector3 &center, const Vector3 &extends);
    void CreateFrustumPointsFromCascadeInterval(float cascadeIntervalBegin, float cascadeIntervalEnd,
                                                const Matrix &projection, Vector3 *pCornerPointsWorld);
    void ComputeNearAndFar(FLOAT &fNearPlane, FLOAT &fFarPlane, DirectX::FXMVECTOR vLightCameraOrthographicMin,
                           DirectX::FXMVECTOR vLightCameraOrthographicMax, Vector3 *pvPointsInCameraView);

  public:
    // shadowHeight == shadowWidth
    BOOL Initialize(D3D12Renderer *pRnd, UINT shadowWidth, UINT cascadedLevel);

    BOOL Add(const RENDER_ITEM *pItem);

    BOOL Update(const BoundingBox *pSceneBox, const Matrix &viewerCameraView, const Matrix &viewerCameraProjection,
                const Matrix &lightCameraView, const Matrix &lightCameraProjection, float nearZ, float farZ);

    TEXTURE_HANDLE *GetShadowMapTexture() const { return m_pCascadedShadowMapTexture; }

    void RenderShadowForAllCascades(UINT threadIndex, CommandListPool* pCommandListPool, ID3D12CommandQueue *pCommandQueue);

    void Reset();

    const Matrix &GetShadowViewMatrix() const { return m_shadowView; }
    const Matrix &GetShadowProjMatrix() const { return m_shadowProj[0]; }

    CascadedShadowsManager() = default;
    ~CascadedShadowsManager();
};
