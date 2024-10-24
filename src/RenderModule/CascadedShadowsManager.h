#pragma once

// reference: DirectX SDK Sample - CascadedShadowMaps11

#include "D3D12Renderer.h"
#include "pch.h"

const UINT MAX_CASADEDS = 8;

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

    Camera *m_pViewerCamera = nullptr;
    Camera *m_pLightCamera = nullptr;

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

    // Debuggin¿ë Texture
    TEXTURE_HANDLE *m_pCascadedShadowMapTexture = nullptr;

    // Config
    BOOL              m_IsMoveLightTexelSize;
    FIT_PROJETION_TO  m_selectedCascadesFit;
    FIT_NEARFAR       m_selectedNearFarFit;
    CAMERA_SELECTION  m_selectedCamera;
    CASCADE_SELECTION m_selectedCascadedSelection;

    int   m_cascadePartitionsMax;
    float m_cascadePartitionsFrustum[MAX_CASADEDS];
    int   m_cascadePartitionsZeroToOne[MAX_CASADEDS]; // Values are  between near and far
    int   m_PCFBlurSize;
    float m_PCFOffset;
    int   m_derivativeBasedOffset;
    int   m_blurBetweenCascades;
    float m_blurBetweenCascadesAmount;

  private:
    void Cleanup();
    void CreateAABBPoints(Vector3 *pAABBPoints, const Vector3 &center, const Vector3 &extends);
    void CreateFrustumPointsFromCascadeInterval(float cascadeIntervalBegin, float cascadeIntervalEnd,
                                                const Matrix &projection, Vector3 *pCornerPointsWorld);
    void ComputeNearAndFar(FLOAT &fNearPlane, FLOAT &fFarPlane, DirectX::FXMVECTOR vLightCameraOrthographicMin,
                           DirectX::FXMVECTOR vLightCameraOrthographicMax, Vector3 *pvPointsInCameraView);

  public:
    // shadowHeight == shadowWidth
    BOOL Initialize(D3D12Renderer *pRnd, Camera *pViewerCamera, Camera *pLightCamera, UINT shadowWidth,
                    UINT cascadedLevel);

    BOOL Update(const BoundingBox *pSceneBox);

    CascadedShadowsManager() = default;
    ~CascadedShadowsManager();
};
