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

void CascadedShadowsManager::CreateAABBPoints(Vector3 *pAABBPoints, const Vector3 &center, const Vector3 &extends)
{
    static const DirectX::XMVECTORF32 extentsMap[] = {
        {1.0f, 1.0f, -1.0f, 1.0f}, {-1.0f, 1.0f, -1.0f, 1.0f}, {1.0f, -1.0f, -1.0f, 1.0f}, {-1.0f, -1.0f, -1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},  {-1.0f, 1.0f, 1.0f, 1.0f},  {1.0f, -1.0f, 1.0f, 1.0f},  {-1.0f, -1.0f, 1.0f, 1.0f}};

    for (UINT i = 0; i < MAX_CASADEDS; i++)
    {
        pAABBPoints[i] = DirectX::XMVectorMultiplyAdd(extentsMap[i], extends, center);
    }
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

BOOL CascadedShadowsManager::Update(const BoundingBox *pSceneBox)
{
    const Matrix &viewerCameraProjection = m_pViewerCamera->GetProjMatrix();
    const Matrix &viewerCameraView = m_pViewerCamera->GetViewMatrix();

    Matrix viewerCameraViewInverse = viewerCameraView.Invert();

    const Matrix &lightCameraProjection = m_pLightCamera->GetProjMatrix();
    const Matrix &lightCameraView = m_pLightCamera->GetViewMatrix();

    Vector3 sceneAABBPointsLightSpace[MAX_CASADEDS];
    // This function simply converts the center and extents of an AABB into 8 points
    CreateAABBPoints(sceneAABBPointsLightSpace, pSceneBox->Center, pSceneBox->Extents);
    // Transform the scene AABB to Light space.
    for (int i = 0; i < MAX_CASADEDS; i++)
    {
        sceneAABBPointsLightSpace[i] = Vector3::Transform(sceneAABBPointsLightSpace[i], lightCameraView);
    }

    float   frustumIntervalBegin, frustumIntervalEnd;
    Vector3 lightCameraOrthographicMin; // light space frustrum aabb
    Vector3 lightCameraOrthographicMax;
    float   cameraNearFarRange = m_pViewerCamera->GetFarClip() - m_pViewerCamera->GetNearClip();

    Vector3 WorldUnitsPerTexel = Vector3::Zero;

    // We loop over the cascades to calculate the orthographic projection for each cascade.
    for (UINT cascadeIndex = 0; cascadeIndex < m_cascadedLevels; cascadeIndex++)
    {
        // Calculate the interval of the View Frustum that this cascade covers. We measure the interval
        // the cascade covers as a Min and Max distance along the Z Axis.
        if (m_selectedCascadesFit == FIT_PROJETION_TO_CASCADES)
        {
            // Because we want to fit the orthogrpahic projection tightly around the Cascade, we set the Mimiumum
            // cascade value to the previous Frustum end Interval
            if (cascadeIndex == 0)
                frustumIntervalBegin = 0.0f;
            else
                frustumIntervalBegin = (FLOAT)m_cascadePartitionsZeroToOne[cascadeIndex - 1];
        }
        else
        {
            // In the FIT_TO_SCENE technique the Cascades overlap eachother.  In other words, interval 1 is coverd by
            // cascades 1 to 8, interval 2 is covered by cascades 2 to 8 and so forth.
            frustumIntervalBegin = 0.0f;
        }

        // Scale the intervals between 0 and 1. They are now percentages that we can scale with.
        frustumIntervalEnd = (FLOAT)m_cascadePartitionsZeroToOne[cascadeIndex];
        frustumIntervalBegin /= (FLOAT)m_cascadePartitionsMax;
        frustumIntervalEnd /= (FLOAT)m_cascadePartitionsMax;
        frustumIntervalBegin = frustumIntervalBegin * cameraNearFarRange;
        frustumIntervalEnd = frustumIntervalEnd * cameraNearFarRange;
        Vector3 frustumPoints[8];

        // This function takes the began and end intervals along with the projection matrix and returns the 8
        // points that repreresent the cascade Interval
        CreateFrustumPointsFromCascadeInterval(frustumIntervalBegin, frustumIntervalEnd, viewerCameraProjection,
                                               frustumPoints);

        lightCameraOrthographicMax = Vector3(FLT_MAX);
        lightCameraOrthographicMin = Vector3(FLT_MIN);

        Vector3 tempTranslatedCornerPoint;
        // This next section of code calculates the min and max values for the orthographic projection.
        for (int icpIndex = 0; icpIndex < MAX_CASADEDS; ++icpIndex)
        {
            // Transform the frustum from camera view space to world space.
            frustumPoints[icpIndex] = Vector3::Transform(frustumPoints[icpIndex], viewerCameraViewInverse);
            // Transform the point from world space to Light Camera Space.
            tempTranslatedCornerPoint = Vector3::Transform(frustumPoints[icpIndex], lightCameraView);
            // Find the closest point.
            lightCameraOrthographicMin = Vector3::Min(tempTranslatedCornerPoint, lightCameraOrthographicMin);
            lightCameraOrthographicMax = Vector3::Max(tempTranslatedCornerPoint, lightCameraOrthographicMax);
        }

        // This code removes the shimmering effect along the edges of shadows due to
        // the light changing to fit the camera.
        if (m_selectedCascadesFit == FIT_PROJETION_TO_SCENE)
        {
            // Fit the ortho projection to the cascades far plane and a near plane of zero.
            // Pad the projection to be the size of the diagonal of the Frustum partition.
            //
            // To do this, we pad the ortho transform so that it is always big enough to cover
            // the entire camera view frustum.
            Vector3 diagonal = frustumPoints[0] - frustumPoints[6];
            diagonal = XMVector3Length(diagonal);

            // The bound is the length of the diagonal of the frustum interval.
            FLOAT cascadeBound = XMVectorGetX(diagonal);

            // The offset calculated will pad the ortho projection so that it is always the same size
            // and big enough to cover the entire cascade interval.
            Vector3 boarderOffset = (diagonal - (lightCameraOrthographicMax - lightCameraOrthographicMin)) * 0.5f;
            // Set the Z and W components to zero.
            boarderOffset.z = 0.0f;

            // Add the offsets to the projection.
            lightCameraOrthographicMax += boarderOffset;
            lightCameraOrthographicMin -= boarderOffset;

            // The world units per texel are used to snap the shadow the orthographic projection
            // to texel sized increments.  This keeps the edges of the shadows from shimmering.
            float worldUnitsPerTexelValue = cascadeBound / (float)m_shadowWidth;
            WorldUnitsPerTexel = XMVectorSet(worldUnitsPerTexelValue, worldUnitsPerTexelValue, 0.0f, 0.0f);
        }
        else if (m_selectedCascadesFit == FIT_PROJETION_TO_CASCADES)
        {

            // We calculate a looser bound based on the size of the PCF blur.  This ensures us that we're
            // sampling within the correct map.
            float   scaleDuetoBlureAMTValue = ((float)(m_PCFBlurSize * 2 + 1) / (float)m_shadowWidth);
            Vector3 scaleDuetoBlureAMT(scaleDuetoBlureAMTValue, scaleDuetoBlureAMTValue, 0.0f);

            float   normalizeByBufferSizeValue = (1.0f / (float)m_shadowWidth);
            Vector3 normalizeByBufferSize(normalizeByBufferSizeValue, normalizeByBufferSizeValue, 0.0f);

            // We calculate the offsets as a percentage of the bound.
            Vector3 boarderOffset = lightCameraOrthographicMax - lightCameraOrthographicMin;
            boarderOffset *= 0.5f;
            boarderOffset *= scaleDuetoBlureAMT;
            lightCameraOrthographicMax += boarderOffset;
            lightCameraOrthographicMin -= boarderOffset;

            // The world units per texel are used to snap  the orthographic projection
            // to texel sized increments.
            // Because we're fitting tighly to the cascades, the shimmering shadow edges will still be present when the
            // camera rotates.  However, when zooming in or strafing the shadow edge will not shimmer.
            WorldUnitsPerTexel = lightCameraOrthographicMax - lightCameraOrthographicMin;
            WorldUnitsPerTexel *= normalizeByBufferSize;
        }
        float lightCameraOrthographicMinZ = lightCameraOrthographicMin.z;

        if (m_IsMoveLightTexelSize)
        {
            // We snape the camera to 1 pixel increments so that moving the camera does not cause the shadows to jitter.
            // This is a matter of integer dividing by the world space size of a texel
            lightCameraOrthographicMin = lightCameraOrthographicMin / WorldUnitsPerTexel;
            lightCameraOrthographicMin = DirectX::XMVectorFloor(lightCameraOrthographicMin);
            lightCameraOrthographicMin *= WorldUnitsPerTexel;

            lightCameraOrthographicMax = lightCameraOrthographicMax / WorldUnitsPerTexel;
            lightCameraOrthographicMax = DirectX::XMVectorFloor(lightCameraOrthographicMax);
            lightCameraOrthographicMax *= WorldUnitsPerTexel;
        }

        // These are the unconfigured near and far plane values.  They are purposly awful to show
        //  how important calculating accurate near and far planes is.
        FLOAT nearPlane = 0.0f;
        FLOAT farPlane = 10000.0f;

        if (m_selectedNearFarFit == FIT_NEARFAR_AABB)
        {
            Vector3 lightSpaceSceneAABBminValue(FLT_MAX); // world space scene aabb
            Vector3 lightSpaceSceneAABBmaxValue(FLT_MIN);
            // We calculate the min and max vectors of the scene in light space. The min and max "Z" values of the
            // light space AABB can be used for the near and far plane. This is easier than intersecting the scene with
            // the AABB and in some cases provides similar results.
            for (int index = 0; index < 8; ++index)
            {
                lightSpaceSceneAABBminValue =
                    Vector3::Min(sceneAABBPointsLightSpace[index], lightSpaceSceneAABBminValue);
                lightSpaceSceneAABBmaxValue =
                    Vector3::Max(sceneAABBPointsLightSpace[index], lightSpaceSceneAABBmaxValue);
            }

            // The min and max z values are the near and far planes.
            nearPlane = lightSpaceSceneAABBminValue.z;
            farPlane = lightSpaceSceneAABBmaxValue.z;
        }
        else if (m_selectedNearFarFit == FIT_NEARFAR_SCENE_AABB || m_selectedNearFarFit == FIT_NEARFAR_PANCAKING)
        {
            // By intersecting the light frustum with the scene AABB we can get a tighter bound on the near and far
            // plane.
            ComputeNearAndFar(nearPlane, farPlane, lightCameraOrthographicMin, lightCameraOrthographicMax,
                              sceneAABBPointsLightSpace);
            if (m_selectedNearFarFit == FIT_NEARFAR_PANCAKING)
            {
                if (lightCameraOrthographicMinZ > nearPlane)
                {
                    nearPlane = lightCameraOrthographicMinZ;
                }
            }
        }
        else
        {
        }
        // Craete the orthographic projection for this cascade.
        m_shadowProj[cascadeIndex] = XMMatrixOrthographicOffCenterLH(
            lightCameraOrthographicMin.x, lightCameraOrthographicMax.x, lightCameraOrthographicMin.y,
            lightCameraOrthographicMax.y, nearPlane, farPlane); 
        
        m_cascadePartitionsFrustum[cascadeIndex] = frustumIntervalEnd;
    }
    m_shadowView = m_pLightCamera->GetViewMatrix();

    return FALSE;
}

void CascadedShadowsManager::CreateFrustumPointsFromCascadeInterval(float cascadeIntervalBegin,
                                                                    float cascadeIntervalEnd, const Matrix &projection,
                                                                    Vector3 *pCornerPointsWorld)
{
    BoundingFrustum viewFrustum(projection, false);
    viewFrustum.Near = cascadeIntervalBegin;
    viewFrustum.Far = cascadeIntervalEnd;

    static const DirectX::XMVECTORU32 vGrabY = {0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000};
    static const DirectX::XMVECTORU32 vGrabX = {0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000};

    Vector3 rightTop(viewFrustum.RightSlope, viewFrustum.TopSlope, 1.0f);
    Vector3 leftBottom(viewFrustum.LeftSlope, viewFrustum.BottomSlope, 1.0f);
    Vector3 rightTopNear = rightTop * viewFrustum.Near;
    Vector3 rightTopFar = rightTop * viewFrustum.Far;
    Vector3 leftBottomNear = leftBottom * viewFrustum.Near;
    Vector3 leftBottomFar = leftBottom * viewFrustum.Far;

    pCornerPointsWorld[0] = rightTopNear;
    pCornerPointsWorld[1] = XMVectorSelect(rightTopNear, leftBottomNear, vGrabX);
    pCornerPointsWorld[2] = leftBottomNear;
    pCornerPointsWorld[3] = XMVectorSelect(rightTopNear, leftBottomNear, vGrabY);

    pCornerPointsWorld[4] = rightTopFar;
    pCornerPointsWorld[5] = XMVectorSelect(rightTopFar, leftBottomFar, vGrabX);
    pCornerPointsWorld[6] = leftBottomFar;
    pCornerPointsWorld[7] = XMVectorSelect(rightTopFar, leftBottomFar, vGrabY);
}


//--------------------------------------------------------------------------------------
// Used to compute an intersection of the orthographic projection and the Scene AABB
//--------------------------------------------------------------------------------------
using namespace DirectX;

struct Triangle
{
    XMVECTOR pt[3];
    BOOL     culled;
};

void CascadedShadowsManager::ComputeNearAndFar(FLOAT &fNearPlane, FLOAT &fFarPlane,
                                               FXMVECTOR vLightCameraOrthographicMin,
                                               FXMVECTOR vLightCameraOrthographicMax, Vector3 *pvPointsInCameraView)
{
    // Initialize the near and far planes
    fNearPlane = FLT_MAX;
    fFarPlane = -FLT_MAX;

    Triangle triangleList[16];
    INT      iTriangleCnt = 1;

    triangleList[0].pt[0] = pvPointsInCameraView[0];
    triangleList[0].pt[1] = pvPointsInCameraView[1];
    triangleList[0].pt[2] = pvPointsInCameraView[2];
    triangleList[0].culled = false;

    // These are the indices used to tesselate an AABB into a list of triangles.
    static const INT iAABBTriIndexes[] = {0, 1, 2, 1, 2, 3, 4, 5, 6, 5, 6, 7, 0, 2, 4, 2, 4, 6,
                                          1, 3, 5, 3, 5, 7, 0, 1, 4, 1, 4, 5, 2, 3, 6, 3, 6, 7};

    INT iPointPassesCollision[3];

    // At a high level:
    // 1. Iterate over all 12 triangles of the AABB.
    // 2. Clip the triangles against each plane. Create new triangles as needed.
    // 3. Find the min and max z values as the near and far plane.

    // This is easier because the triangles are in camera spacing making the collisions tests simple comparisions.

    float fLightCameraOrthographicMinX = XMVectorGetX(vLightCameraOrthographicMin);
    float fLightCameraOrthographicMaxX = XMVectorGetX(vLightCameraOrthographicMax);
    float fLightCameraOrthographicMinY = XMVectorGetY(vLightCameraOrthographicMin);
    float fLightCameraOrthographicMaxY = XMVectorGetY(vLightCameraOrthographicMax);

    for (INT AABBTriIter = 0; AABBTriIter < 12; ++AABBTriIter)
    {

        triangleList[0].pt[0] = pvPointsInCameraView[iAABBTriIndexes[AABBTriIter * 3 + 0]];
        triangleList[0].pt[1] = pvPointsInCameraView[iAABBTriIndexes[AABBTriIter * 3 + 1]];
        triangleList[0].pt[2] = pvPointsInCameraView[iAABBTriIndexes[AABBTriIter * 3 + 2]];
        iTriangleCnt = 1;
        triangleList[0].culled = FALSE;

        // Clip each invidual triangle against the 4 frustums.  When ever a triangle is clipped into new triangles,
        // add them to the list.
        for (INT frustumPlaneIter = 0; frustumPlaneIter < 4; ++frustumPlaneIter)
        {

            FLOAT fEdge;
            INT   iComponent;

            if (frustumPlaneIter == 0)
            {
                fEdge = fLightCameraOrthographicMinX; // todo make float temp
                iComponent = 0;
            }
            else if (frustumPlaneIter == 1)
            {
                fEdge = fLightCameraOrthographicMaxX;
                iComponent = 0;
            }
            else if (frustumPlaneIter == 2)
            {
                fEdge = fLightCameraOrthographicMinY;
                iComponent = 1;
            }
            else
            {
                fEdge = fLightCameraOrthographicMaxY;
                iComponent = 1;
            }

            for (INT triIter = 0; triIter < iTriangleCnt; ++triIter)
            {
                // We don't delete triangles, so we skip those that have been culled.
                if (!triangleList[triIter].culled)
                {
                    INT      iInsideVertCount = 0;
                    XMVECTOR tempOrder;
                    // Test against the correct frustum plane.
                    // This could be written more compactly, but it would be harder to understand.

                    if (frustumPlaneIter == 0)
                    {
                        for (INT triPtIter = 0; triPtIter < 3; ++triPtIter)
                        {
                            if (XMVectorGetX(triangleList[triIter].pt[triPtIter]) >
                                XMVectorGetX(vLightCameraOrthographicMin))
                            {
                                iPointPassesCollision[triPtIter] = 1;
                            }
                            else
                            {
                                iPointPassesCollision[triPtIter] = 0;
                            }
                            iInsideVertCount += iPointPassesCollision[triPtIter];
                        }
                    }
                    else if (frustumPlaneIter == 1)
                    {
                        for (INT triPtIter = 0; triPtIter < 3; ++triPtIter)
                        {
                            if (XMVectorGetX(triangleList[triIter].pt[triPtIter]) <
                                XMVectorGetX(vLightCameraOrthographicMax))
                            {
                                iPointPassesCollision[triPtIter] = 1;
                            }
                            else
                            {
                                iPointPassesCollision[triPtIter] = 0;
                            }
                            iInsideVertCount += iPointPassesCollision[triPtIter];
                        }
                    }
                    else if (frustumPlaneIter == 2)
                    {
                        for (INT triPtIter = 0; triPtIter < 3; ++triPtIter)
                        {
                            if (XMVectorGetY(triangleList[triIter].pt[triPtIter]) >
                                XMVectorGetY(vLightCameraOrthographicMin))
                            {
                                iPointPassesCollision[triPtIter] = 1;
                            }
                            else
                            {
                                iPointPassesCollision[triPtIter] = 0;
                            }
                            iInsideVertCount += iPointPassesCollision[triPtIter];
                        }
                    }
                    else
                    {
                        for (INT triPtIter = 0; triPtIter < 3; ++triPtIter)
                        {
                            if (XMVectorGetY(triangleList[triIter].pt[triPtIter]) <
                                XMVectorGetY(vLightCameraOrthographicMax))
                            {
                                iPointPassesCollision[triPtIter] = 1;
                            }
                            else
                            {
                                iPointPassesCollision[triPtIter] = 0;
                            }
                            iInsideVertCount += iPointPassesCollision[triPtIter];
                        }
                    }

                    // Move the points that pass the frustum test to the begining of the array.
                    if (iPointPassesCollision[1] && !iPointPassesCollision[0])
                    {
                        tempOrder = triangleList[triIter].pt[0];
                        triangleList[triIter].pt[0] = triangleList[triIter].pt[1];
                        triangleList[triIter].pt[1] = tempOrder;
                        iPointPassesCollision[0] = TRUE;
                        iPointPassesCollision[1] = FALSE;
                    }
                    if (iPointPassesCollision[2] && !iPointPassesCollision[1])
                    {
                        tempOrder = triangleList[triIter].pt[1];
                        triangleList[triIter].pt[1] = triangleList[triIter].pt[2];
                        triangleList[triIter].pt[2] = tempOrder;
                        iPointPassesCollision[1] = TRUE;
                        iPointPassesCollision[2] = FALSE;
                    }
                    if (iPointPassesCollision[1] && !iPointPassesCollision[0])
                    {
                        tempOrder = triangleList[triIter].pt[0];
                        triangleList[triIter].pt[0] = triangleList[triIter].pt[1];
                        triangleList[triIter].pt[1] = tempOrder;
                        iPointPassesCollision[0] = TRUE;
                        iPointPassesCollision[1] = FALSE;
                    }

                    if (iInsideVertCount == 0)
                    { // All points failed. We're done,
                        triangleList[triIter].culled = true;
                    }
                    else if (iInsideVertCount == 1)
                    { // One point passed. Clip the triangle against the Frustum plane
                        triangleList[triIter].culled = false;

                        //
                        XMVECTOR vVert0ToVert1 = triangleList[triIter].pt[1] - triangleList[triIter].pt[0];
                        XMVECTOR vVert0ToVert2 = triangleList[triIter].pt[2] - triangleList[triIter].pt[0];

                        // Find the collision ratio.
                        FLOAT fHitPointTimeRatio = fEdge - XMVectorGetByIndex(triangleList[triIter].pt[0], iComponent);
                        // Calculate the distance along the vector as ratio of the hit ratio to the component.
                        FLOAT fDistanceAlongVector01 =
                            fHitPointTimeRatio / XMVectorGetByIndex(vVert0ToVert1, iComponent);
                        FLOAT fDistanceAlongVector02 =
                            fHitPointTimeRatio / XMVectorGetByIndex(vVert0ToVert2, iComponent);
                        // Add the point plus a percentage of the vector.
                        vVert0ToVert1 *= fDistanceAlongVector01;
                        vVert0ToVert1 += triangleList[triIter].pt[0];
                        vVert0ToVert2 *= fDistanceAlongVector02;
                        vVert0ToVert2 += triangleList[triIter].pt[0];

                        triangleList[triIter].pt[1] = vVert0ToVert2;
                        triangleList[triIter].pt[2] = vVert0ToVert1;
                    }
                    else if (iInsideVertCount == 2)
                    { // 2 in  // tesselate into 2 triangles

                        // Copy the triangle\(if it exists) after the current triangle out of
                        // the way so we can override it with the new triangle we're inserting.
                        triangleList[iTriangleCnt] = triangleList[triIter + 1];

                        triangleList[triIter].culled = false;
                        triangleList[triIter + 1].culled = false;

                        // Get the vector from the outside point into the 2 inside points.
                        XMVECTOR vVert2ToVert0 = triangleList[triIter].pt[0] - triangleList[triIter].pt[2];
                        XMVECTOR vVert2ToVert1 = triangleList[triIter].pt[1] - triangleList[triIter].pt[2];

                        // Get the hit point ratio.
                        FLOAT fHitPointTime_2_0 = fEdge - XMVectorGetByIndex(triangleList[triIter].pt[2], iComponent);
                        FLOAT fDistanceAlongVector_2_0 =
                            fHitPointTime_2_0 / XMVectorGetByIndex(vVert2ToVert0, iComponent);
                        // Calcaulte the new vert by adding the percentage of the vector plus point 2.
                        vVert2ToVert0 *= fDistanceAlongVector_2_0;
                        vVert2ToVert0 += triangleList[triIter].pt[2];

                        // Add a new triangle.
                        triangleList[triIter + 1].pt[0] = triangleList[triIter].pt[0];
                        triangleList[triIter + 1].pt[1] = triangleList[triIter].pt[1];
                        triangleList[triIter + 1].pt[2] = vVert2ToVert0;

                        // Get the hit point ratio.
                        FLOAT fHitPointTime_2_1 = fEdge - XMVectorGetByIndex(triangleList[triIter].pt[2], iComponent);
                        FLOAT fDistanceAlongVector_2_1 =
                            fHitPointTime_2_1 / XMVectorGetByIndex(vVert2ToVert1, iComponent);
                        vVert2ToVert1 *= fDistanceAlongVector_2_1;
                        vVert2ToVert1 += triangleList[triIter].pt[2];
                        triangleList[triIter].pt[0] = triangleList[triIter + 1].pt[1];
                        triangleList[triIter].pt[1] = triangleList[triIter + 1].pt[2];
                        triangleList[triIter].pt[2] = vVert2ToVert1;
                        // Cncrement triangle count and skip the triangle we just inserted.
                        ++iTriangleCnt;
                        ++triIter;
                    }
                    else
                    { // all in
                        triangleList[triIter].culled = false;
                    }
                } // end if !culled loop
            }
        }
        for (INT index = 0; index < iTriangleCnt; ++index)
        {
            if (!triangleList[index].culled)
            {
                // Set the near and far plan and the min and max z values respectivly.
                for (int vertind = 0; vertind < 3; ++vertind)
                {
                    float fTriangleCoordZ = XMVectorGetZ(triangleList[index].pt[vertind]);
                    if (fNearPlane > fTriangleCoordZ)
                    {
                        fNearPlane = fTriangleCoordZ;
                    }
                    if (fFarPlane < fTriangleCoordZ)
                    {
                        fFarPlane = fTriangleCoordZ;
                    }
                }
            }
        }
    }
}
