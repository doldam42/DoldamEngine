#include "pch.h"

#include "CommandListPool.h"
#include "ConstantBufferPool.h"
#include "D3D12Renderer.h"
#include "D3D12ResourceManager.h"
#include "D3DMeshObject.h"
#include "DescriptorPool.h"
#include "TextureManager.h"

#include "ShadowManager.h"

using namespace DirectX;

//--------------------------------------------------------------------------------------
// Used to compute an intersection of the orthographic projection and the Scene AABB
//--------------------------------------------------------------------------------------
struct Triangle
{
    XMVECTOR pt[3];
    BOOL     culled;
};

void ShadowManager::Cleanup()
{
    D3D12ResourceManager *pResourceManager = m_pRenderer->INL_GetResourceManager();
    TextureManager       *pTexManager = m_pRenderer->INL_GetTextureManager();

    pResourceManager->DeallocDescriptorTable(&m_shadowMapSRV);
    pResourceManager->DeallocRTVDescriptorTable(&m_shadowMapRTV);
    pResourceManager->DeallocDSVDescriptorTable(&m_shadowMapDSV);

    if (m_pShadowMapTexture)
    {
        pTexManager->DeleteTexture(m_pShadowMapTexture);
        m_pShadowMapTexture = nullptr;
    }

    if (m_pShadowDepthStencils)
    {
        m_pShadowDepthStencils->Release();
        m_pShadowDepthStencils = nullptr;
    }

    if (m_pRenderQueue)
    {
        delete m_pRenderQueue;
        m_pRenderQueue = nullptr;
    }
}

void ShadowManager::UpdateGlobalConstants()
{

    ConstantBufferPool *pCBPool = m_pRenderer->GetConstantBufferPool(CONSTANT_BUFFER_TYPE_GLOBAL, 0);

    m_pShadowGlobalCB = pCBPool->Alloc();

    m_shadowGlobalConsts.view = m_shadowView.Transpose();
    m_shadowGlobalConsts.proj = m_shadowProj.Transpose();
    m_shadowGlobalConsts.viewProj = (m_shadowView * m_shadowProj).Transpose();

    memcpy(m_pShadowGlobalCB->pSystemMemAddr, &m_shadowGlobalConsts, sizeof(GlobalConstants));
}

void ShadowManager::CreateAABBPoints(Vector3 *pOutAABBPoints, const Vector3 &center, const Vector3 &extends)
{
    // This map enables us to use a for loop and do vector math.

    static const XMVECTOR vExtentsMap[] = {
        {1.0f, 1.0f, -1.0f, 1.0f}, {-1.0f, 1.0f, -1.0f, 1.0f}, {1.0f, -1.0f, -1.0f, 1.0f}, {-1.0f, -1.0f, -1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},  {-1.0f, 1.0f, 1.0f, 1.0f},  {1.0f, -1.0f, 1.0f, 1.0f},  {-1.0f, -1.0f, 1.0f, 1.0f}};

    for (INT index = 0; index < 8; ++index)
    {
        pOutAABBPoints[index] = XMVectorMultiplyAdd(vExtentsMap[index], center, extends);
    }
}

void ShadowManager::ComputeNearAndFar(FLOAT &fNearPlane, FLOAT &fFarPlane,
                                      DirectX::FXMVECTOR vLightCameraOrthographicMin,
                                      DirectX::FXMVECTOR vLightCameraOrthographicMax, Vector3 *pvPointsInCameraView)
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

BOOL ShadowManager::Initialize(D3D12Renderer *pRnd, UINT shadowWidth)
{
    BOOL result = FALSE;

    ID3D12Device         *pD3DDevice = pRnd->INL_GetD3DDevice();
    D3D12ResourceManager *pResourceManager = pRnd->INL_GetResourceManager();
    TextureManager       *pTextureManager = pRnd->INL_GetTextureManager();

    m_shadowWidth = shadowWidth;

    // Init
    m_shadowScissorRects.left = 0;
    m_shadowScissorRects.top = 0;
    m_shadowScissorRects.right = shadowWidth;
    m_shadowScissorRects.bottom = shadowWidth;

    m_shadowViewports.TopLeftX = 0;
    m_shadowViewports.TopLeftY = 0;
    m_shadowViewports.Width = float(shadowWidth);
    m_shadowViewports.Height = float(shadowWidth);
    m_shadowViewports.MinDepth = 0.0f;
    m_shadowViewports.MaxDepth = 1.0f;

    pResourceManager->AllocDSVDescriptorTable(&m_shadowMapDSV);
    pResourceManager->AllocRTVDescriptorTable(&m_shadowMapRTV);
    pResourceManager->AllocDescriptorTable(&m_shadowMapSRV, 1);

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
            &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32_TYPELESS, shadowWidth, shadowWidth, 1, 1, 1, 0,
                                          D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
            D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, &depthOptimizedClearValue, IID_PPV_ARGS(&pDepthStencil))))
    {
        __debugbreak();
    }
    pDepthStencil->SetName(L"Cascaded Shadow Map DepthStencil");

    pD3DDevice->CreateShaderResourceView(pDepthStencil, &srvDesc, m_shadowMapSRV.cpuHandle);
    pD3DDevice->CreateDepthStencilView(pDepthStencil, &dsvDesc, m_shadowMapDSV.cpuHandle);

    m_pShadowMapTexture =
        pTextureManager->CreateRenderableTexture(shadowWidth, shadowWidth, DXGI_FORMAT_R8G8B8A8_UNORM);
    pD3DDevice->CreateRenderTargetView(m_pShadowMapTexture->pTexture, nullptr, m_shadowMapRTV.cpuHandle);

    m_pShadowDepthStencils = pDepthStencil;

    m_pRenderQueue = new RenderQueue;
    m_pRenderQueue->Initialize(pRnd, MESH_OBJECT_COUNT_FOR_SHADOW);

    result = TRUE;
lb_return:

    m_pRenderer = pRnd;

    return result;
}

// Shadow는 MeshObject만 그릴 수 있음
BOOL ShadowManager::Add(const RENDER_ITEM *pItem)
{
    Matrix worldTM;
    switch (pItem->type)
    {
    case RENDER_ITEM_TYPE_MESH_OBJ:
        worldTM = pItem->meshObjParam.worldTM;
        break;
    case RENDER_ITEM_TYPE_CHAR_OBJ:
        worldTM = pItem->charObjParam.worldTM;
        break;
    default:
        __debugbreak();
        return FALSE;
    }

    D3DMeshObject     *pMeshObject = (D3DMeshObject *)pItem->pObjHandle;
    const BoundingBox &box = pMeshObject->GetBoundingBox();

    BoundingBox worldBox;
    box.Transform(worldBox, worldTM);
    Vector3 center = worldBox.Center;
    Vector3 minCorner = center - worldBox.Extents;
    Vector3 maxCorner = center + worldBox.Extents;

    m_sceneMinCorner = Vector3::Min(m_sceneMinCorner, minCorner);
    m_sceneMaxCorner = Vector3::Max(m_sceneMaxCorner, maxCorner);

    return m_pRenderQueue->Add(pItem);
}

BOOL ShadowManager::Update(const Matrix &lightCameraView, const Matrix &viewerCameraView,
                           const Matrix &viewerCameraProjection, float nearZ, float farZ)
{
    Vector3 center = (m_sceneMaxCorner + m_sceneMinCorner) * 0.5f;
    Vector3 extends = (m_sceneMaxCorner - m_sceneMinCorner) * 0.5f;

    Matrix viewerCameraViewInverse = viewerCameraView.Invert();

    FLOAT   fFrustumIntervalBegin, fFrustumIntervalEnd;
    Vector3 lightCameraOrthographicMin = Vector3(FLT_MAX); // light space frustrum aabb
    Vector3 lightCameraOrthographicMax = Vector3(FLT_MIN);
    FLOAT   fCameraNearFarRange = farZ - nearZ;

    // Scene의 AABB 포인트들을 구하기
    Vector3 sceneAABBPointsLightSpace[8];
    CreateAABBPoints(sceneAABBPointsLightSpace, center, extends);
    // Transform the scene AABB to Light space.
    for (int index = 0; index < 8; ++index)
    {
        sceneAABBPointsLightSpace[index] = Vector3::Transform(sceneAABBPointsLightSpace[index], lightCameraView);
    }

    // 프러스텀 포인트들을 계산하기
    XMFLOAT3        frustumPoints[8];
    BoundingFrustum frustum(viewerCameraProjection);
    frustum.GetCorners(frustumPoints);

    Vector3 tempTranslatedCornerPoint;
    // This next section of code calculates the min and max values for the orthographic projection.
    for (int icpIndex = 0; icpIndex < 8; ++icpIndex)
    {
        // Transform the frustum from camera view space to world space.
        frustumPoints[icpIndex] = Vector3::Transform(frustumPoints[icpIndex], viewerCameraViewInverse);
        // Transform the point from world space to Light Camera Space.
        tempTranslatedCornerPoint = Vector3::Transform(frustumPoints[icpIndex], lightCameraView);
        // Find the closest point.
        lightCameraOrthographicMin = Vector3::Min(tempTranslatedCornerPoint, lightCameraOrthographicMin);
        lightCameraOrthographicMax = Vector3::Max(tempTranslatedCornerPoint, lightCameraOrthographicMax);
    }

    // These are the unconfigured near and far plane values.  They are purposly awful to show
    //  how important calculating accurate near and far planes is.
    FLOAT nearPlane = 0.0f;
    FLOAT farPlane = 10000.0f;

    ComputeNearAndFar(nearPlane, farPlane, lightCameraOrthographicMin, lightCameraOrthographicMax,
                      sceneAABBPointsLightSpace);

    // Craete the orthographic projection for this cascade.
  /*  m_shadowProj = XMMatrixOrthographicOffCenterLH(lightCameraOrthographicMin.x, lightCameraOrthographicMax.x,
                                                   lightCameraOrthographicMin.y, lightCameraOrthographicMax.y,
                                                   nearPlane, farPlane);*/
    m_shadowProj = XMMatrixOrthographicOffCenterLH(lightCameraOrthographicMin.x, lightCameraOrthographicMax.x,
                                                   lightCameraOrthographicMin.y, lightCameraOrthographicMax.y,
                                                   nearPlane, farPlane);

    m_shadowView = lightCameraView;

    UpdateGlobalConstants();

    m_sceneMaxCorner = Vector3(FLT_MIN);
    m_sceneMinCorner = Vector3(FLT_MAX);

    return TRUE;
}

void ShadowManager::Render(ID3D12CommandQueue *pCommandQueue)
{
    const UINT threadIndex = 0;

    ID3D12Device              *pD3DDevice = m_pRenderer->INL_GetD3DDevice();
    CommandListPool           *pCommandListPool = m_pRenderer->INL_GetCommandListPool(threadIndex);
    DescriptorPool            *pDescriptorPool = m_pRenderer->INL_GetDescriptorPool(threadIndex);
    ID3D12GraphicsCommandList *pCommandList = pCommandListPool->GetCurrentCommandList();
    ID3D12DescriptorHeap      *pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();

    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_shadowMapDSV.cpuHandle);
    CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_shadowMapSRV.cpuHandle);
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_shadowMapRTV.cpuHandle);

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    pDescriptorPool->Alloc(&cpuHandle, &gpuHandle, 1);
    pD3DDevice->CopyDescriptorsSimple(1, cpuHandle, m_pShadowGlobalCB->CBVHandle,
                                      D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pShadowDepthStencils,
                                                                           D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
                                                                           D3D12_RESOURCE_STATE_DEPTH_WRITE));

    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pShadowMapTexture->pTexture,
                                                                           D3D12_RESOURCE_STATE_COMMON,
                                                                           D3D12_RESOURCE_STATE_RENDER_TARGET));

    float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    pCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    pCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    m_pRenderQueue->Process(threadIndex, pCommandListPool, pCommandQueue, 400, rtvHandle, dsvHandle, gpuHandle,
                            &m_shadowViewports, &m_shadowScissorRects, 1, DRAW_PASS_TYPE_SHADOW);

    pCommandList = pCommandListPool->GetCurrentCommandList();

    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pShadowMapTexture->pTexture,
                                                                           D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                                           D3D12_RESOURCE_STATE_COMMON));
    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pShadowDepthStencils,
                                                                           D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                                           D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE));
    pCommandListPool->CloseAndExecute(pCommandQueue);
}

void ShadowManager::Reset() { m_pRenderQueue->Reset(); }

ShadowManager::~ShadowManager() { Cleanup(); }
