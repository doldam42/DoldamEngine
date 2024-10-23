#pragma once
#include "D3D12Renderer.h"
#include "pch.h"

const UINT MAX_CASADEDS = 8;

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

    Vector3 m_sceneAABBMin;
    Vector3 m_sceneAABBMax;

    Matrix m_shadowProj[MAX_CASADEDS];
    Matrix m_shadowView;

    ID3D12Resource   *m_pCascadedShadowDepthStencils = nullptr;
    DESCRIPTOR_HANDLE m_CascadedShadowMapRTV;
    DESCRIPTOR_HANDLE m_CascadedShadowMapSRV;
    DESCRIPTOR_HANDLE m_CascadedShadowMapDSV;

    // Debuggin¿ë Texture
    TEXTURE_HANDLE *m_pCascadedShadowMapTexture = nullptr;

  private:
    void Cleanup();

//    void CreateAABBPoints(Vector4* pAABBPoints, Vector4 center, )

  public:
    // shadowHeight == shadowWidth
    BOOL Initialize(D3D12Renderer *pRnd, Camera *pViewerCamera, Camera *pLightCamera, UINT shadowWidth, UINT cascadedLevel);

    BOOL Update();

    CascadedShadowsManager() = default;
    ~CascadedShadowsManager();
};
