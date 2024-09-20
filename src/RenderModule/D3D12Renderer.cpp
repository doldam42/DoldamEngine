#include "pch.h"

#include "../GenericModule/ProcessInfo.h"
#include "../GenericModule/StringUtil.h"
#include "ConstantBuffers.h"
#include "CommandListPool.h"
#include "ConstantBuffers.h"
#include "ConstantBufferManager.h"
#include "ConstantBufferPool.h"
#include "Cubemap.h"
#include "D3D12ResourceManager.h"
#include "DXRSceneManager.h"
#include "DescriptorPool.h"
#include "FontManager.h"
#include "GraphicsCommon.h"
#include "MaterialManager.h"
#include "MeshObject.h"
#include "RenderThread.h"
#include "SpriteObject.h"
#include "TextureManager.h"

#include <process.h>

#include "D3D12Renderer.h"

BOOL D3D12Renderer::Initialize(HWND hWnd, BOOL bEnableDebugLayer, BOOL bEnableGBV)
{
    BOOL result = FALSE;
    m_DPI = GetDpiForWindow(hWnd);
    RECT rect;
    ::GetClientRect(hWnd, &rect);
    UINT dwWndWidth = rect.right - rect.left;
    UINT dwWndHeight = rect.bottom - rect.top;
    UINT dwBackBufferWidth = rect.right - rect.left;
    UINT dwBackBufferHeight = rect.bottom - rect.top;

    m_Viewport.Width = (float)dwWndWidth;
    m_Viewport.Height = (float)dwWndHeight;
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;

    m_ScissorRect.left = 0;
    m_ScissorRect.top = 0;
    m_ScissorRect.right = dwWndWidth;
    m_ScissorRect.bottom = dwWndHeight;

    HRESULT            hr = S_OK;
    ID3D12Debug       *pDebugController = nullptr;
    IDXGIFactory4     *pFactory = nullptr;
    IDXGIAdapter1     *pAdapter = nullptr;
    DXGI_ADAPTER_DESC1 AdapterDesc = {};

    UINT dwCreateFlags = 0;
    UINT dwCreateFactoryFlags = 0;

    // if use debug Layer...
    if (bEnableDebugLayer)
    {
        // Enable the D3D12 debug layer.
        hr = D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController));
        if (SUCCEEDED(hr))
        {
            pDebugController->EnableDebugLayer();
        }
        dwCreateFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
        if (bEnableGBV)
        {
            ID3D12Debug5 *pDebugController5 = nullptr;
            if (S_OK == pDebugController->QueryInterface(IID_PPV_ARGS(&pDebugController5)))
            {
                pDebugController5->SetEnableGPUBasedValidation(TRUE);
                pDebugController5->SetEnableAutoName(TRUE);
                pDebugController5->Release();
            }
        }
    }

    // Create DXGIFactory
    CreateDXGIFactory2(dwCreateFactoryFlags, IID_PPV_ARGS(&pFactory));

    D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0,
                                         D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};

    UINT FeatureLevelNum = _countof(featureLevels);

    for (UINT featerLevelIndex = 0; featerLevelIndex < FeatureLevelNum; featerLevelIndex++)
    {
        UINT adapterIndex = 0;
        while (DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &pAdapter))
        {
            pAdapter->GetDesc1(&AdapterDesc);

            if (AdapterDesc.DeviceId != 5567 &&
                SUCCEEDED(D3D12CreateDevice(pAdapter, featureLevels[featerLevelIndex], IID_PPV_ARGS(&m_pD3DDevice))))
            {
                goto lb_exit;
            }
            pAdapter->Release();
            pAdapter = nullptr;
            adapterIndex++;
        }
    }

lb_exit:
    if (!m_pD3DDevice)
    {
        __debugbreak();
        goto lb_return;
    }

    m_AdapterDesc = AdapterDesc;
    m_hWnd = hWnd;

    if (pDebugController)
    {
        SetDebugLayerInfo(m_pD3DDevice);
    }

    // Describe and create the command queue.
    {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        hr = m_pD3DDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue));
        if (FAILED(hr))
        {
            __debugbreak();
            goto lb_return;
        }
    }

    // Describe and create the swap chain.
    {
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = dwBackBufferWidth;
        swapChainDesc.Height = dwBackBufferHeight;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        // swapChainDesc.BufferDesc.RefreshRate.Numerator = m_uiRefreshRate;
        // swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = SWAP_CHAIN_FRAME_COUNT;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.Scaling = DXGI_SCALING_NONE;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        swapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
        m_dwSwapChainFlags = swapChainDesc.Flags;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
        fsSwapChainDesc.Windowed = TRUE;

        IDXGISwapChain1 *pSwapChain1 = nullptr;
        if (FAILED(pFactory->CreateSwapChainForHwnd(m_pCommandQueue, hWnd, &swapChainDesc, &fsSwapChainDesc, nullptr,
                                                    &pSwapChain1)))
        {
            __debugbreak();
        }
        pSwapChain1->QueryInterface(IID_PPV_ARGS(&m_pSwapChain));
        pSwapChain1->Release();
        pSwapChain1 = nullptr;
        m_uiFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
    }

    CreateDescriptorHeap();

    CreateBuffers();

    CreateFence();

    m_pFontManager = new FontManager;
    m_pFontManager->Initialize(this, m_pCommandQueue, 1024, 256, bEnableDebugLayer);

    Graphics::InitCommonStates(m_pD3DDevice);

    m_pResourceManager = new D3D12ResourceManager;
    m_pResourceManager->Initialize(m_pD3DDevice, MAX_DESCRIPTOR_COUNT);

    m_pTextureManager = new TextureManager;
    m_pTextureManager->Initialize(this, 1024 / 16, 1024);

    m_pMaterialManager = new MaterialManager;
    m_pMaterialManager->Initialize(this, sizeof(MaterialConstants), 1024);

    DWORD physicalCoreCount = 0;
    DWORD logicalCoreCount = 0;
    TryGetPhysicalCoreCount(&physicalCoreCount, &logicalCoreCount);
    m_renderThreadCount = (UINT)physicalCoreCount;
    if (m_renderThreadCount > MAX_RENDER_THREAD_COUNT)
        m_renderThreadCount = MAX_RENDER_THREAD_COUNT;

#ifdef USE_MULTI_THREAD
    InitRenderThreadPool(m_renderThreadCount);
#endif

    for (UINT i = 0; i < MAX_PENDING_FRAME_COUNT; i++)
    {
        for (UINT j = 0; j < m_renderThreadCount; j++)
        {
            m_ppCommandListPool[i][j] = new CommandListPool;
            m_ppCommandListPool[i][j]->Initialize(m_pD3DDevice, D3D12_COMMAND_LIST_TYPE_DIRECT, 2048);

            m_ppDescriptorPool[i][j] = new DescriptorPool;
            m_ppDescriptorPool[i][j]->Initialize(m_pD3DDevice, MAX_DRAW_COUNT_PER_FRAME *
                                                                   MeshObject::MAX_DESCRIPTOR_COUNT_PER_DRAW_STATIC);

            m_ppConstantBufferManager[i][j] = new ConstantBufferManager;
            m_ppConstantBufferManager[i][j]->Initialize(m_pD3DDevice, MAX_DRAW_COUNT_PER_FRAME);
        }
    }

    for (UINT i = 0; i < m_renderThreadCount; i++)
    {
        m_ppRenderQueue[i] = new RenderQueue;
        m_ppRenderQueue[i]->Initialize(this, MAX_DRAW_COUNT_PER_FRAME);
    }

    m_pNonOpaqueRenderQueue = new RenderQueue;
    m_pNonOpaqueRenderQueue->Initialize(this, MAX_DRAW_COUNT_PER_FRAME);

// #DXR
#ifdef USE_RAYTRACING
    m_pDXRSceneManager = new DXRSceneManager;
    ID3D12GraphicsCommandList4 *pCommandList = m_ppCommandListPool[m_dwCurContextIndex][0]->GetCurrentCommandList();
    m_pDXRSceneManager->Initialize(this, pCommandList, MAX_DRAW_COUNT_PER_FRAME);
    m_ppCommandListPool[m_dwCurContextIndex][0]->CloseAndExecute(m_pCommandQueue);
#endif
    CreateDefaultTex();

    result = TRUE;

lb_return:
    if (pDebugController)
    {
        pDebugController->Release();
        pDebugController = nullptr;
    }
    if (pAdapter)
    {
        pAdapter->Release();
        pAdapter = nullptr;
    }
    if (pFactory)
    {
        pFactory->Release();
        pFactory = nullptr;
    }
    return result;
}

void D3D12Renderer::BeginRender()
{

    CommandListPool           *pCommandListPool = m_ppCommandListPool[m_dwCurContextIndex][0];
    ID3D12GraphicsCommandList *pCommandList = pCommandListPool->GetCurrentCommandList();

    m_pMaterialManager->Update(pCommandList);

    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pIntermediateRenderTargets[m_uiFrameIndex],
                                                                           D3D12_RESOURCE_STATE_PRESENT,
                                                                           D3D12_RESOURCE_STATE_RENDER_TARGET));
    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets[m_uiFrameIndex],
                                                                           D3D12_RESOURCE_STATE_PRESENT,
                                                                           D3D12_RESOURCE_STATE_RENDER_TARGET));

    D3D12_CPU_DESCRIPTOR_HANDLE   rtvHandle = GetRTVHandle(RENDER_TARGET_TYPE_INTERMEDIATE);
    D3D12_CPU_DESCRIPTOR_HANDLE   backBufferRTV = GetRTVHandle(RENDER_TARGET_TYPE_BACK);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_pDSVHeap->GetCPUDescriptorHandleForHeapStart());
    // Record commands.
    pCommandList->ClearRenderTargetView(rtvHandle, m_clearColor, 0, nullptr);
    pCommandList->ClearRenderTargetView(backBufferRTV, m_clearColor, 0, nullptr);
    pCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    if (m_pCubemap)
    {
        pCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
        pCommandList->RSSetViewports(1, &m_Viewport);
        pCommandList->RSSetScissorRects(1, &m_ScissorRect);
        m_pCubemap->Draw(0, pCommandList);
    }

    pCommandListPool->CloseAndExecute(m_pCommandQueue);
}

void D3D12Renderer::EndRender()
{
    CommandListPool            *pCommadListPool = m_ppCommandListPool[m_dwCurContextIndex][0];
    ID3D12GraphicsCommandList4 *pCommandList = pCommadListPool->GetCurrentCommandList();

    D3D12_CPU_DESCRIPTOR_HANDLE   rtvHandle = GetRTVHandle(RENDER_TARGET_TYPE_INTERMEDIATE);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_pDSVHeap->GetCPUDescriptorHandleForHeapStart());
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtHandle(m_pRaytracingOutputHeap->GetCPUDescriptorHandleForHeapStart(),
                                           m_uiFrameIndex, m_rtvDescriptorSize);

#if defined(USE_MULTI_THREAD)
    m_activeThreadCount = m_renderThreadCount;
    for (UINT i = 0; i < m_renderThreadCount; i++)
    {
        SetEvent(m_pThreadDescList[i].hEventList[RENDER_THREAD_EVENT_TYPE_PROCESS]);
    }
    WaitForSingleObject(m_hCompleteEvent, INFINITE);

    m_pNonOpaqueRenderQueue->Process(0, pCommadListPool, m_pCommandQueue, 400, rtvHandle, dsvHandle, &m_Viewport,
                                     &m_ScissorRect);

#elif defined(USE_RAYTRACING)
    m_pDXRSceneManager->CreateTopLevelAS(pCommandList);

    ID3D12Resource *pOutputView = m_pRTOutputBuffers[m_uiFrameIndex];
    const float     clearColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
    pCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_pDXRSceneManager->DispatchRay(pCommandList, pOutputView, rtOutputHandle);
    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets[m_uiFrameIndex],
                                                                           D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                                           D3D12_RESOURCE_STATE_COPY_DEST));
    pCommandList->CopyResource(m_pRenderTargets[m_uiFrameIndex], pOutputView);

    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets[m_uiFrameIndex],
                                                                           D3D12_RESOURCE_STATE_COPY_DEST,
                                                                           D3D12_RESOURCE_STATE_RENDER_TARGET));
#else
    for (UINT i = 0; i < m_renderThreadCount; i++)
    {
        pCommadListPool = m_ppCommandListPool[m_dwCurContextIndex][i];
        m_ppRenderQueue[i]->Process(i, pCommadListPool, m_pCommandQueue, 400, rtvHandle, dsvHandle, &m_Viewport,
                                    &m_ScissorRect);
    }
#endif
    pCommandList = pCommadListPool->GetCurrentCommandList();

    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pIntermediateRenderTargets[m_uiFrameIndex],
                                                                           D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                                           D3D12_RESOURCE_STATE_PRESENT));

    // TODO: HDR
    D3D12_CPU_DESCRIPTOR_HANDLE   backBufferRTV = GetRTVHandle(RENDER_TARGET_TYPE_BACK);
    CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(m_pSRVHeap->GetGPUDescriptorHandleForHeapStart(), m_uiFrameIndex,
                                            m_srvDescriptorSize);

    pCommandList->RSSetViewports(1, &m_Viewport);
    pCommandList->RSSetScissorRects(1, &m_ScissorRect);
    pCommandList->OMSetRenderTargets(1, &backBufferRTV, FALSE, nullptr);

    pCommandList->SetGraphicsRootSignature(Graphics::presentRS);
    pCommandList->SetDescriptorHeaps(1, &m_pSRVHeap);
    pCommandList->SetGraphicsRootDescriptorTable(0, srvHandle);
    pCommandList->SetPipelineState(Graphics::presentPSO);
    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCommandList->IASetVertexBuffers(0, 1, &SpriteObject::GetVertexBufferView());
    pCommandList->IASetIndexBuffer(&SpriteObject::GetIndexBufferView());
    pCommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets[m_uiFrameIndex],
                                                                           D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                                           D3D12_RESOURCE_STATE_PRESENT));
    pCommadListPool->CloseAndExecute(m_pCommandQueue);

    for (UINT i = 0; i < m_renderThreadCount; i++)
    {
        m_ppRenderQueue[i]->Reset();
    }
    m_pNonOpaqueRenderQueue->Reset();

#ifdef USE_RAYTRACING
    m_pDXRSceneManager->Reset();
#endif
}

void D3D12Renderer::Present()
{
    Fence();

    //
    // Back Buffer 화면을 Primary Buffer로 전송
    //
    // UINT m_SyncInterval = 1; // VSync On
    UINT m_SyncInterval = 0; // VSync Off

    UINT uiSyncInterval = m_SyncInterval;
    UINT uiPresentFlags = 0;

    if (!uiSyncInterval)
    {
        uiPresentFlags = DXGI_PRESENT_ALLOW_TEARING;
    }

    HRESULT hr = m_pSwapChain->Present(uiSyncInterval, uiPresentFlags);

    if (DXGI_ERROR_DEVICE_REMOVED == hr)
    {
        __debugbreak();
    }

    // for next frame
    m_uiFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

    // prepare next frame
    UINT dwNextContextIndex = (m_dwCurContextIndex + 1) % MAX_PENDING_FRAME_COUNT;
    WaitForFenceValue(m_pui64LastFenceValue[dwNextContextIndex]);

    for (UINT i = 0; i < m_renderThreadCount; i++)
    {
        // reset resources per frame
        m_ppConstantBufferManager[dwNextContextIndex][i]->Reset();
        m_ppDescriptorPool[dwNextContextIndex][i]->Reset();
        m_ppCommandListPool[dwNextContextIndex][i]->Reset();
    }
    m_dwCurContextIndex = dwNextContextIndex;
}

void D3D12Renderer::OnUpdateWindowSize(UINT width, UINT height)
{
    Fence();
    // wait for all commands
    for (UINT i = 0; i < MAX_PENDING_FRAME_COUNT; i++)
    {
        WaitForFenceValue(m_ui64FenceValue);
    }

    BOOL result = FALSE;

    if (width == 0 || height == 0)
        __debugbreak();

    m_Viewport.Width = (float)width;
    m_Viewport.Height = (float)height;

    m_ScissorRect.right = width;
    m_ScissorRect.bottom = height;

    CleanupBuffers();

    if (FAILED(m_pSwapChain->ResizeBuffers(SWAP_CHAIN_FRAME_COUNT, width, height, DXGI_FORMAT_R8G8B8A8_UNORM,
                                           m_dwSwapChainFlags)))
    {
        __debugbreak();
    }

    m_uiFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

    CreateBuffers();
}

IRenderMesh *D3D12Renderer::CreateSkinnedObject()
{
    MeshObject *pMeshObj = new MeshObject;

    pMeshObj->Initialize(this, RENDER_ITEM_TYPE_CHAR_OBJ);
    pMeshObj->AddRef();
    return pMeshObj;
}

IRenderMesh *D3D12Renderer::CreateMeshObject()
{
    MeshObject *pMeshObj = new MeshObject;

    pMeshObj->Initialize(this, RENDER_ITEM_TYPE_MESH_OBJ);
    pMeshObj->AddRef();
    return pMeshObj;
}

IRenderSprite *D3D12Renderer::CreateSpriteObject()
{
    SpriteObject *pSprObj = new SpriteObject;
    pSprObj->Initialize(this);
    pSprObj->AddRef();
    return pSprObj;
}

IRenderSprite *D3D12Renderer::CreateSpriteObject(const WCHAR *texFileName, int PosX, int PosY, int Width, int Height)
{
    SpriteObject *pSprObj = new SpriteObject;

    RECT rect;
    rect.left = PosX;
    rect.top = PosY;
    rect.right = Width;
    rect.bottom = Height;
    pSprObj->Initialize(this, texFileName, &rect);
    pSprObj->AddRef();
    return pSprObj;
}

void D3D12Renderer::RenderMeshObject(IRenderMesh *pMeshObj, const Matrix *pWorldMat, bool isWired, UINT numInstance)
{
    // CommandListPool            *pCommadListPool = m_ppCommandListPool[m_dwCurContextIndex][0];
    // ID3D12GraphicsCommandList4 *pCommandList = pCommadListPool->GetCurrentCommandList();
    MeshObject *pMeshObject = static_cast<MeshObject *>(pMeshObj);
#ifdef USE_RAYTRACING

    UINT numGeometry = pMeshObject->GetGeometryCount();

    Graphics::LOCAL_ROOT_ARG *pRootArgs = pMeshObject->GetRootArgs();

    m_pDXRSceneManager->InsertInstance(pMeshObject->GetBottomLevelAS(), pWorldMat, 0, pRootArgs, numGeometry);
#else
    RENDER_ITEM item = {};
    item.type = RENDER_ITEM_TYPE_MESH_OBJ;
    item.pObjHandle = (void *)pMeshObj;
    item.meshObjParam.fillMode = isWired ? FILL_MODE_WIRED : FILL_MODE_SOLID;
    item.meshObjParam.worldTM = (*pWorldMat);

    if (pMeshObject->GetPassType() == DRAW_PASS_TYPE_NON_OPAQUE)
    {
        if (!m_pNonOpaqueRenderQueue->Add(&item))
            __debugbreak();
    }
    else
    {
        if (!m_ppRenderQueue[m_curThreadIndex]->Add(&item))
            __debugbreak();
    }

    m_curThreadIndex++;
    m_curThreadIndex = m_curThreadIndex % m_renderThreadCount;
#endif //  USE_RAYTRACING
}

void D3D12Renderer::RenderCharacterObject(IRenderMesh *pCharObj, const Matrix *pWorldMat, const Matrix *pBoneMats,
                                          UINT numBones, bool isWired)
{
    // ID3D12GraphicsCommandList *pCommandList = m_ppCommandList[m_dwCurContextIndex];
#ifdef USE_RAYTRACING
    CommandListPool            *pCommadListPool = m_ppCommandListPool[m_dwCurContextIndex][0];
    ID3D12GraphicsCommandList4 *pCommandList = pCommadListPool->GetCurrentCommandList();
    MeshObject                 *pMeshObject = (MeshObject *)pCharObj;

    pMeshObject->UpdateSkinnedBLAS(pCommandList, pBoneMats, numBones);
    // pCommadListPool->CloseAndExecute(m_pCommandQueue);
    UINT numGeometry = pMeshObject->GetGeometryCount();

    Graphics::LOCAL_ROOT_ARG *pRootArgs = pMeshObject->GetRootArgs();

    m_pDXRSceneManager->InsertInstance(pMeshObject->GetBottomLevelAS(), pWorldMat, 0, pRootArgs, numGeometry);
#else
    RENDER_ITEM item = {};
    item.type = RENDER_ITEM_TYPE_CHAR_OBJ;
    item.pObjHandle = pCharObj;
    item.charObjParam = {};
    item.charObjParam.fillMode = isWired ? FILL_MODE_WIRED : FILL_MODE_SOLID;
    item.charObjParam.worldTM = (*pWorldMat);
    item.charObjParam.pBones = pBoneMats;
    item.charObjParam.numBones = numBones;

    if (!m_ppRenderQueue[m_curThreadIndex]->Add(&item))
        __debugbreak();

    m_curThreadIndex++;
    m_curThreadIndex = m_curThreadIndex % m_renderThreadCount;
#endif
}

void D3D12Renderer::RenderSpriteWithTex(IRenderSprite *pSprObjHandle, int iPosX, int iPosY, float fScaleX,
                                        float fScaleY, const RECT *pRect, float Z, void *pTexHandle)
{
    // ID3D12GraphicsCommandList *pCommandList = m_ppCommandList[m_dwCurContextIndex];

    RENDER_ITEM item = {};
    item.type = RENDER_ITEM_TYPE_SPRITE;
    item.pObjHandle = (void *)pSprObjHandle;
    item.spriteParam.posX = iPosX;
    item.spriteParam.posY = iPosY;
    item.spriteParam.scaleX = fScaleX;
    item.spriteParam.scaleY = fScaleY;

    if (pRect)
    {
        item.spriteParam.isUseRect = TRUE;
        item.spriteParam.rect = *pRect;
    }
    else
    {
        item.spriteParam.isUseRect = FALSE;
        item.spriteParam.rect = {};
    }
    item.spriteParam.pTexHandle = pTexHandle;
    item.spriteParam.Z = Z;

    if (!m_ppRenderQueue[m_curThreadIndex]->Add(&item))
        __debugbreak();

    m_curThreadIndex++;
    m_curThreadIndex = m_curThreadIndex % m_renderThreadCount;
}

void D3D12Renderer::RenderSprite(IRenderSprite *pSprObjHandle, int iPosX, int iPosY, float fScaleX, float fScaleY,
                                 float Z)
{
    // ID3D12GraphicsCommandList *pCommandList = m_ppCommandList[m_dwCurContextIndex];

    RENDER_ITEM item = {};
    item.type = RENDER_ITEM_TYPE_SPRITE;
    item.pObjHandle = (void *)pSprObjHandle;
    item.spriteParam.posX = iPosX;
    item.spriteParam.posY = iPosY;
    item.spriteParam.scaleX = fScaleX;
    item.spriteParam.scaleY = fScaleY;
    item.spriteParam.isUseRect = FALSE;
    item.spriteParam.rect = {};
    item.spriteParam.pTexHandle = nullptr;
    item.spriteParam.Z = Z;

    if (!m_ppRenderQueue[m_curThreadIndex]->Add(&item))
        __debugbreak();

    m_curThreadIndex++;
    m_curThreadIndex = m_curThreadIndex % m_renderThreadCount;
}

IFontHandle *D3D12Renderer::CreateFontObject(const WCHAR *fontFamilyName, float fontSize)
{
    FONT_HANDLE *pFontHandle = m_pFontManager->CreateFontObject(fontFamilyName, fontSize);
    return pFontHandle;
}

void D3D12Renderer::DeleteFontObject(IFontHandle *pFontHandle)
{
    m_pFontManager->DeleteFontObject(static_cast<FONT_HANDLE *>(pFontHandle));
}

BOOL D3D12Renderer::WriteTextToBitmap(BYTE *pDestImage, UINT destWidth, UINT destHeight, UINT destPitch, int *pOutWidth,
                                      int *pOutHeight, IFontHandle *pFontObjHandle, const WCHAR *inStr, UINT len)
{
    BOOL result = m_pFontManager->WriteTextToBitmap(pDestImage, destWidth, destHeight, destPitch, pOutWidth, pOutHeight,
                                                    static_cast<FONT_HANDLE *>(pFontObjHandle), inStr, len);

    return result;
}

BOOL D3D12Renderer::BeginCreateMesh(IRenderMesh *pMeshObjHandle, const void *pVertices, UINT numVertices,
                                    UINT numFaceGroup, const wchar_t *path)
{
    MeshObject *pMeshObj = dynamic_cast<MeshObject *>(pMeshObjHandle);
    BOOL        result = pMeshObj->BeginCreateMesh(pVertices, numVertices, numFaceGroup, path);
    return result;
}

BOOL D3D12Renderer::InsertFaceGroup(IRenderMesh *pMeshObjHandle, const UINT *pIndices, UINT numTriangles,
                                    const Material *pInMaterial)
{
    MeshObject *pMeshObj = dynamic_cast<MeshObject *>(pMeshObjHandle);
    BOOL        result = pMeshObj->InsertFaceGroup(pIndices, numTriangles, pInMaterial);

    return result;
}

void D3D12Renderer::EndCreateMesh(IRenderMesh *pMeshObjHandle)
{
    CommandListPool            *pCommandListPool = m_ppCommandListPool[m_dwCurContextIndex][0];
    ID3D12GraphicsCommandList4 *pCommandList = pCommandListPool->GetCurrentCommandList();
    MeshObject                 *pMeshObj = dynamic_cast<MeshObject *>(pMeshObjHandle);
    pMeshObj->EndCreateMesh(pCommandList);
    pCommandListPool->CloseAndExecute(m_pCommandQueue);
}

void D3D12Renderer::UpdateCamera(const Vector3 &eyeWorld, const Matrix &viewRow, const Matrix &projRow)
{
    m_pGlobalCB = GetConstantBufferPool(CONSTANT_BUFFER_TYPE_GLOBAL, 0)->Alloc();

    GlobalConstants globalConsts = m_globalConsts;
    globalConsts.eyeWorld = eyeWorld;
    globalConsts.view = viewRow.Transpose();
    globalConsts.proj = projRow.Transpose();
    globalConsts.invView = viewRow.Invert().Transpose();
    globalConsts.invProj = projRow.Invert().Transpose();
    globalConsts.viewProj = (viewRow * projRow).Transpose();
    globalConsts.invViewProj = (viewRow * projRow).Invert().Transpose();
    memcpy(&globalConsts.lights, m_pLights, sizeof(Light) * MAX_LIGHTS);

    memcpy(m_pGlobalCB->pSystemMemAddr, &globalConsts, sizeof(GlobalConstants));
}

void D3D12Renderer::UpdateTextureWithImage(ITextureHandle *pTexHandle, const BYTE *pSrcBits, UINT srcWidth,
                                           UINT srcHeight)
{
    TEXTURE_HANDLE *pTextureHandle = static_cast<TEXTURE_HANDLE *>(pTexHandle);
    ID3D12Resource *pDestTexResource = pTextureHandle->pTexture;
    ID3D12Resource *pUploadBuffer = pTextureHandle->pUploadBuffer;

    D3D12_RESOURCE_DESC desc = pDestTexResource->GetDesc();
    if (srcWidth > desc.Width)
    {
        __debugbreak();
    }
    if (srcHeight > desc.Height)
    {
        __debugbreak();
    }
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint;
    UINT                               Rows = 0;
    UINT64                             RowSize = 0;
    UINT64                             TotalBytes = 0;

    m_pD3DDevice->GetCopyableFootprints(&desc, 0, 1, 0, &Footprint, &Rows, &RowSize, &TotalBytes);

    BYTE         *pMappedPtr = nullptr;
    CD3DX12_RANGE writeRange(0, 0);

    HRESULT hr = pUploadBuffer->Map(0, &writeRange, reinterpret_cast<void **>(&pMappedPtr));
    if (FAILED(hr))
        __debugbreak();

    const BYTE *pSrc = pSrcBits;
    BYTE       *pDest = pMappedPtr;
    for (UINT y = 0; y < srcHeight; y++)
    {
        memcpy(pDest, pSrc, srcWidth * 4);
        pSrc += (srcWidth * 4);
        pDest += Footprint.Footprint.RowPitch;
    }
    // Unmap
    pUploadBuffer->Unmap(0, nullptr);

    pTextureHandle->IsUpdated = TRUE;
}

ITextureHandle *D3D12Renderer::CreateTiledTexture(UINT texWidth, UINT texHeight, UINT r, UINT g, UINT b)
{
    DXGI_FORMAT TexFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    BYTE *pImage = (BYTE *)malloc(texWidth * texHeight * 4);
    memset(pImage, 0, texWidth * texHeight * 4);

    BOOL bFirstColorIsWhite = TRUE;

    for (UINT y = 0; y < texHeight; y++)
    {
        for (UINT x = 0; x < texWidth; x++)
        {
            RGBA *pDest = (RGBA *)(pImage + (x + y * texWidth) * 4);

            if ((bFirstColorIsWhite + x) % 2)
            {
                pDest->r = r;
                pDest->g = g;
                pDest->b = b;
            }
            else
            {
                pDest->r = 0;
                pDest->g = 0;
                pDest->b = 0;
            }
            pDest->a = 255;
        }
        bFirstColorIsWhite++;
        bFirstColorIsWhite %= 2;
    }
    TEXTURE_HANDLE *pTexHandle = m_pTextureManager->CreateImmutableTexture(texWidth, texHeight, TexFormat, pImage);
    free(pImage);
    pImage = nullptr;

    return pTexHandle;
}

ITextureHandle *D3D12Renderer::CreateDynamicTexture(UINT texWidth, UINT texHeight)
{
    TEXTURE_HANDLE *pTexHandle = m_pTextureManager->CreateDynamicTexture(texWidth, texHeight);
    return pTexHandle;
}

/*
 * 지원 포맷
 * .DDS /.BMP / .PNG / .GIF / .TIFF / .JPEG
 */
ITextureHandle *D3D12Renderer::CreateTextureFromFile(const WCHAR *fileName)
{
    TEXTURE_HANDLE *pTexHandle = m_pTextureManager->CreateTextureFromFile(fileName, false);
    return pTexHandle;
}

ITextureHandle *D3D12Renderer::CreateCubemapFromFile(const WCHAR *fileName)
{
    TEXTURE_HANDLE *pTexHandle = m_pTextureManager->CreateTextureFromFile(fileName, true);
    return pTexHandle;
}

ITextureHandle *D3D12Renderer::CreateMetallicRoughnessTexture(const WCHAR *metallicFilename,
                                                              const WCHAR *roughneessFilename)
{
    TEXTURE_HANDLE *pTexHandle =
        m_pTextureManager->CreateMetallicRoughnessTexture(metallicFilename, roughneessFilename);
    return pTexHandle;
}

void D3D12Renderer::DeleteTexture(ITextureHandle *pTexHandle)
{
    // wait for all commands
    for (UINT i = 0; i < MAX_PENDING_FRAME_COUNT; i++)
    {
        WaitForFenceValue(m_pui64LastFenceValue[i]);
    }

    m_pTextureManager->DeleteTexture(static_cast<TEXTURE_HANDLE *>(pTexHandle));
}

void *D3D12Renderer::CreateDirectionalLight(const Vector3 *pRadiance, const Vector3 *pDirection)
{
    Light light;
    light.direction = *pDirection;
    light.radiance = *pRadiance;
    light.type = LIGHT_DIRECTIONAL;

    for (UINT i = 0; i < MAX_LIGHTS; i++)
    {
        Light *pLight = m_pLights + i;
        if (pLight->type == LIGHT_OFF)
        {
            *pLight = light;
            return pLight;
        }
    }
    return nullptr;
}

void *D3D12Renderer::CreatePointLight(const Vector3 *pRadiance, const Vector3 *pDirection, const Vector3 *pPosition,
                                      float radius, float fallOffStart, float fallOffEnd)
{
    Light light;
    light.direction = *pDirection;
    light.radiance = *pRadiance;
    light.position = *pPosition;
    light.radius = radius;
    light.fallOffStart = fallOffStart;
    light.fallOffEnd = fallOffEnd;

    light.type = LIGHT_POINT;

    for (UINT i = 0; i < MAX_LIGHTS; i++)
    {
        Light *pLight = m_pLights + i;
        if (pLight->type == LIGHT_OFF)
        {
            *pLight = light;
            return pLight;
        }
    }
    return nullptr;
}

void *D3D12Renderer::CreateSpotLight(const Vector3 *pRadiance, const Vector3 *pDirection, const Vector3 *pPosition,
                                     float spotPower, float radius, float fallOffStart, float fallOffEnd)
{
    Light light;
    light.radiance = *pRadiance;
    light.direction = *pDirection;
    light.position = *pPosition;
    light.spotPower = spotPower;
    light.radius = radius;
    light.fallOffStart = fallOffStart;
    light.fallOffEnd = fallOffEnd;

    light.type = LIGHT_SPOT;

    for (UINT i = 0; i < MAX_LIGHTS; i++)
    {
        Light *pLight = m_pLights + i;
        if (pLight->type == LIGHT_OFF)
        {
            *pLight = light;
            return pLight;
        }
    }
    return nullptr;
}

void D3D12Renderer::DeleteLight(void *pLightHandle)
{
    // Boundary Check
    if (pLightHandle < m_pLights || pLightHandle >= ((uint8_t *)m_pLights + sizeof(Light) * MAX_LIGHTS))
    {
        __debugbreak();
    }
    Light *pLight = (Light *)pLightHandle;
    pLight->type = LIGHT_OFF;
}

IMaterialHandle *D3D12Renderer::CreateMaterialHandle(const Material *pInMaterial)
{
    MaterialConstants mat;

    mat.albedo = pInMaterial->albedo;
    mat.emissive = pInMaterial->emissive;
    mat.metallicFactor = pInMaterial->metallicFactor;
    mat.roughnessFactor = pInMaterial->roughnessFactor;
    mat.tansparancy = pInMaterial->tansparancy;
    mat.reflection = pInMaterial->reflection;

    mat.useAlbedoMap = wcslen(pInMaterial->albedoTextureName) == 0 ? FALSE : TRUE;
    mat.useAOMap = wcslen(pInMaterial->aoTextureName) == 0 ? FALSE : TRUE;
    mat.useEmissiveMap = wcslen(pInMaterial->emissiveTextureName) == 0 ? FALSE : TRUE;
    mat.useMetallicMap = wcslen(pInMaterial->metallicTextureName) == 0 ? FALSE : TRUE;
    mat.useRoughnessMap = mat.useMetallicMap;
    mat.useNormalMap = wcslen(pInMaterial->normalTextureName) == 0 ? FALSE : TRUE;

    MATERIAL_HANDLE *pMatHandle = m_pMaterialManager->CreateMaterial(&mat, pInMaterial->name);
    return pMatHandle;
}

void D3D12Renderer::DeleteMaterialHandle(IMaterialHandle *pInMaterial)
{
    MATERIAL_HANDLE *pHandle = static_cast<MATERIAL_HANDLE *>(pInMaterial);
    m_pMaterialManager->DeleteMaterial(pHandle);
}

void D3D12Renderer::UpdateMaterialHandle(IMaterialHandle *pInMaterial, const Material *pMaterial) 
{
    m_pMaterialManager->UpdateMaterial(static_cast<MATERIAL_HANDLE*>(pInMaterial), pMaterial);
}

void D3D12Renderer::InitCubemaps(const WCHAR *envFilename, const WCHAR *specularFilename,
                                 const WCHAR *irradianceFilename, const WCHAR *brdfFilename)
{
    m_pCubemap = new Cubemap;

    m_pCubemap->Initialize(this, envFilename, specularFilename, irradianceFilename, brdfFilename);
}

UINT D3D12Renderer::GetCommandListCount()
{
    UINT commandListCount = 0;
    for (UINT i = 0; i < MAX_PENDING_FRAME_COUNT; i++)
    {
        for (UINT j = 0; j < m_renderThreadCount; j++)
        {
            commandListCount += m_ppCommandListPool[i][j]->GetTotalCmdListNum();
        }
    }
    return commandListCount;
}

TEXTURE_HANDLE *D3D12Renderer::GetDefaultTex()
{
    m_pDefaultTexHandle->refCount++;
    return m_pDefaultTexHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE D3D12Renderer::GetGlobalDescriptorHandle(UINT threadIndex)
{
    // | Global Constants(b0) | Materials(t5) | IBL Textures(t10~14) |
    DescriptorPool *pDescriptorPool = INL_GetDescriptorPool(threadIndex);

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    pDescriptorPool->Alloc(&cpuHandle, &gpuHandle, GLOBAL_DESCRIPTOR_INDEX_COUNT);

    CD3DX12_CPU_DESCRIPTOR_HANDLE cbHandle(cpuHandle, GLOBAL_DESCRIPTOR_INDEX_CB, m_srvDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE matHandle(cpuHandle, GLOBAL_DESCRIPTOR_INDEX_MATERIALS, m_srvDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE envHandle(cpuHandle, GLOBAL_DESCRIPTOR_INDEX_IBL_ENV, m_srvDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE specularHandle(cpuHandle, GLOBAL_DESCRIPTOR_INDEX_IBL_SPECULAR, m_srvDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE irradianceHandle(cpuHandle, GLOBAL_DESCRIPTOR_INDEX_IBL_IRRADIANCE,
                                                   m_srvDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE brdfHandle(cpuHandle, GLOBAL_DESCRIPTOR_INDEX_IBL_BRDF, m_srvDescriptorSize);

    m_pD3DDevice->CopyDescriptorsSimple(1, cbHandle, m_pGlobalCB->CBVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_pD3DDevice->CopyDescriptorsSimple(1, matHandle, m_pMaterialManager->GetSRV(),
                                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_pD3DDevice->CopyDescriptorsSimple(1, envHandle, m_pCubemap->GetEnvSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_pD3DDevice->CopyDescriptorsSimple(1, specularHandle, m_pCubemap->GetSpecularSRV(),
                                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_pD3DDevice->CopyDescriptorsSimple(1, irradianceHandle, m_pCubemap->GetIrradianceSRV(),
                                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_pD3DDevice->CopyDescriptorsSimple(1, brdfHandle, m_pCubemap->GetBrdfSRV(),
                                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    return gpuHandle;
}

ConstantBufferPool *D3D12Renderer::GetConstantBufferPool(CONSTANT_BUFFER_TYPE type, UINT threadIndex)
{
    return m_ppConstantBufferManager[m_dwCurContextIndex][threadIndex]->GetConstantBufferPool(type);
}

// RTV handle
// |  backBuffer rtv0  |  backBuffer rtv1  | ... |
// | intermediate rtv0 | intermediate rtv1 | ... |

D3D12_CPU_DESCRIPTOR_HANDLE D3D12Renderer::GetRTVHandle(RENDER_TARGET_TYPE type) const
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_pRTVHeap->GetCPUDescriptorHandleForHeapStart(),
                                         type * SWAP_CHAIN_FRAME_COUNT, m_rtvDescriptorSize);
    return handle.Offset(m_uiFrameIndex, m_rtvDescriptorSize);
}

void D3D12Renderer::ProcessByThread(UINT threadIndex)
{
    CommandListPool *pCommandListPool = m_ppCommandListPool[m_dwCurContextIndex][threadIndex];

    D3D12_CPU_DESCRIPTOR_HANDLE   rtvHandle = GetRTVHandle(RENDER_TARGET_TYPE_INTERMEDIATE);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_pDSVHeap->GetCPUDescriptorHandleForHeapStart());

    m_ppRenderQueue[threadIndex]->Process(threadIndex, pCommandListPool, m_pCommandQueue, 400, rtvHandle, dsvHandle,
                                          &m_Viewport, &m_ScissorRect);

    LONG curCount = _InterlockedDecrement(&m_activeThreadCount);
    if (curCount == 0)
    {
        SetEvent(m_hCompleteEvent);
    }
}

void D3D12Renderer::WaitForGPU()
{
    for (UINT i = 0; i < MAX_PENDING_FRAME_COUNT; i++)
    {
        WaitForFenceValue(m_pui64LastFenceValue[i]);
    }
}

D3D12Renderer::~D3D12Renderer() { Cleanup(); }

void D3D12Renderer::CreateDefaultTex()
{
    DXGI_FORMAT TexFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    UINT        width = 256;
    UINT        height = 256;
    UINT        channel = 4;

    BYTE *defaultTex = new BYTE[width * height * channel];
    memset(defaultTex, 0, sizeof(BYTE) * width * height * channel);

    m_pDefaultTexHandle = m_pTextureManager->CreateImmutableTexture(width, height, TexFormat, defaultTex);

    delete[] defaultTex;
    defaultTex = nullptr;
}

void D3D12Renderer::CreateFence()
{
    // Create synchronization objects and wait until assets have been uploaded to
    // the GPU.
    if (FAILED(m_pD3DDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence))))
    {
        __debugbreak();
    }

    m_ui64FenceValue = 0;

    // Create an event handle to use for frame synchronization.
    m_hFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

void D3D12Renderer::CleanupFence()
{
    if (m_hFenceEvent)
    {
        CloseHandle(m_hFenceEvent);
        m_hFenceEvent = nullptr;
    }
    if (m_pFence)
    {
        m_pFence->Release();
        m_pFence = nullptr;
    }
}

BOOL D3D12Renderer::CreateDescriptorHeap()
{
    HRESULT hr = S_OK;

    // 렌더타겟용 디스크립터힙
    // |       rtv 0       |       rtv 1       | ... |
    // | intermediate rtv0 | intermediate rtv1 | ... |
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = SWAP_CHAIN_FRAME_COUNT * RENDER_TARGET_TYPE_COUNT;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(m_pD3DDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_pRTVHeap))))
    {
        __debugbreak();
    }

    // 렌더타겟 SRV용 디스크립터 힙
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = SWAP_CHAIN_FRAME_COUNT; // SwapChain Buffers * render target type
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (FAILED(m_pD3DDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_pSRVHeap))))
    {
        __debugbreak();
    }

    // 깁이 버퍼용 디스크립터 힙
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(m_pD3DDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_pDSVHeap))))
    {
        __debugbreak();
    }

    // Raytracing용 디스크립터 힙
    D3D12_DESCRIPTOR_HEAP_DESC rtHeapDesc = {};
    rtHeapDesc.NumDescriptors = SWAP_CHAIN_FRAME_COUNT;
    rtHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(m_pD3DDevice->CreateDescriptorHeap(&rtHeapDesc, IID_PPV_ARGS(&m_pRaytracingOutputHeap))))
    {
        __debugbreak();
    }

    // Descriptor Size 저장
    m_rtvDescriptorSize = m_pD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_srvDescriptorSize = m_pD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    return TRUE;
}

void D3D12Renderer::CleanupDescriptorHeap()
{
    if (m_pRTVHeap)
    {
        m_pRTVHeap->Release();
        m_pRTVHeap = nullptr;
    }

    if (m_pDSVHeap)
    {
        m_pDSVHeap->Release();
        m_pDSVHeap = nullptr;
    }

    if (m_pSRVHeap)
    {
        m_pSRVHeap->Release();
        m_pSRVHeap = nullptr;
    }

    if (m_pRaytracingOutputHeap)
    {
        m_pRaytracingOutputHeap->Release();
        m_pRaytracingOutputHeap = nullptr;
    }
}

UINT64 D3D12Renderer::Fence()
{
    m_ui64FenceValue++;
    m_pCommandQueue->Signal(m_pFence, m_ui64FenceValue);
    m_pui64LastFenceValue[m_dwCurContextIndex] = m_ui64FenceValue;
    return m_ui64FenceValue;
}

void D3D12Renderer::WaitForFenceValue(UINT64 ExpectedFenceValue)
{

    // Wait until the previous frame is finished.
    if (m_pFence->GetCompletedValue() < ExpectedFenceValue)
    {
        m_pFence->SetEventOnCompletion(ExpectedFenceValue, m_hFenceEvent);
        WaitForSingleObject(m_hFenceEvent, INFINITE);
    }
}

void D3D12Renderer::CreateBuffers()
{
    // Create frame resources.

    // Create a RTV for each frame.
    // Descriptor Table
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_pRTVHeap->GetCPUDescriptorHandleForHeapStart());
    CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_pSRVHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT n = 0; n < SWAP_CHAIN_FRAME_COUNT; n++)
    {
        m_pSwapChain->GetBuffer(n, IID_PPV_ARGS(&m_pRenderTargets[n]));
        m_pD3DDevice->CreateRenderTargetView(m_pRenderTargets[n], nullptr, rtvHandle);

        rtvHandle.Offset(1, m_rtvDescriptorSize);
    }

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    clearValue.Color[0] = m_clearColor[0];
    clearValue.Color[1] = m_clearColor[1];
    clearValue.Color[2] = m_clearColor[2];
    clearValue.Color[3] = m_clearColor[3];

    for (UINT n = 0; n < SWAP_CHAIN_FRAME_COUNT; n++)
    {
        if (FAILED(m_pD3DDevice->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16G16B16A16_FLOAT, m_Viewport.Width, m_Viewport.Height, 1, 1,
                                              1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
                D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(&m_pIntermediateRenderTargets[n]))))
        {
            __debugbreak();
        }
        m_pD3DDevice->CreateRenderTargetView(m_pIntermediateRenderTargets[n], nullptr, rtvHandle);
        m_pD3DDevice->CreateShaderResourceView(m_pIntermediateRenderTargets[n], nullptr, srvHandle);
        rtvHandle.Offset(1, m_rtvDescriptorSize);
        srvHandle.Offset(1, m_srvDescriptorSize);
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtHandle(m_pRaytracingOutputHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT n = 0; n < SWAP_CHAIN_FRAME_COUNT; n++)
    {
        ID3D12Resource *pOutBuffer = nullptr;
        if (FAILED(m_pD3DDevice->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, m_Viewport.Width, m_Viewport.Height, 1, 1, 1,
                                              0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
                D3D12_RESOURCE_STATE_COPY_SOURCE, nullptr, IID_PPV_ARGS(&pOutBuffer))))
        {
            __debugbreak();
        }

        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        m_pD3DDevice->CreateUnorderedAccessView(pOutBuffer, nullptr, &uavDesc, rtHandle);

        m_pRTOutputBuffers[n] = pOutBuffer;
        rtHandle.Offset(1, m_srvDescriptorSize);
    }

    // Create DSV
    {
        D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
        depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
        depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

        D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
        depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
        depthOptimizedClearValue.DepthStencil.Stencil = 0;

        if (FAILED(m_pD3DDevice->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_Viewport.Width, m_Viewport.Height, 1, 0, 1, 0,
                                              D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
                D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthOptimizedClearValue, IID_PPV_ARGS(&m_pDepthStencil))))
        {
            __debugbreak();
        }

        m_pD3DDevice->CreateDepthStencilView(m_pDepthStencil, &depthStencilDesc,
                                             m_pDSVHeap->GetCPUDescriptorHandleForHeapStart());
    }
}

void D3D12Renderer::CleanupBuffers()
{
    // Cleanup Render Targets
    for (UINT i = 0; i < SWAP_CHAIN_FRAME_COUNT; i++)
    {
        if (m_pRenderTargets[i])
        {
            m_pRenderTargets[i]->Release();
            m_pRenderTargets[i] = nullptr;
        }

        if (m_pIntermediateRenderTargets[i])
        {
            m_pIntermediateRenderTargets[i]->Release();
            m_pIntermediateRenderTargets[i] = nullptr;
        }

        if (m_pRTOutputBuffers[i])
        {
            m_pRTOutputBuffers[i]->Release();
            m_pRTOutputBuffers[i] = nullptr;
        }
    }

    if (m_pDepthStencil)
    {
        m_pDepthStencil->Release();
        m_pDepthStencil = nullptr;
    }
}

void D3D12Renderer::Cleanup()
{
#ifdef USE_MULTI_THREAD
    CleanupRenderThreadPool();
#endif

    Fence();

    WaitForFenceValue(m_ui64FenceValue);

    if (m_pDefaultTexHandle)
    {
        DeleteTexture(m_pDefaultTexHandle);
        m_pDefaultTexHandle = nullptr;
    }

    if (m_pCubemap)
    {
        delete m_pCubemap;
        m_pCubemap = nullptr;
    }

    if (m_pDXRSceneManager)
    {
        delete m_pDXRSceneManager;
        m_pDXRSceneManager = nullptr;
    }

    Graphics::DeleteCommonStates();

    for (UINT i = 0; i < m_renderThreadCount; i++)
    {
        if (m_ppRenderQueue[i])
        {
            delete m_ppRenderQueue[i];
            m_ppRenderQueue[i] = nullptr;
        }
    }

    if (m_pNonOpaqueRenderQueue)
    {
        delete m_pNonOpaqueRenderQueue;
        m_pNonOpaqueRenderQueue = nullptr;
    }

    for (UINT i = 0; i < MAX_PENDING_FRAME_COUNT; i++)
    {
        for (UINT j = 0; j < m_renderThreadCount; j++)
        {
            if (m_ppCommandListPool[i][j])
            {
                delete m_ppCommandListPool[i][j];
                m_ppCommandListPool[i][j] = nullptr;
            }

            if (m_ppConstantBufferManager[i][j])
            {
                delete m_ppConstantBufferManager[i][j];
                m_ppConstantBufferManager[i][j] = nullptr;
            }

            if (m_ppDescriptorPool[i][j])
            {
                delete m_ppDescriptorPool[i][j];
                m_ppDescriptorPool[i][j] = nullptr;
            }
        }
    }

    if (m_pSwapChain)
    {
        m_pSwapChain->Release();
        m_pSwapChain = nullptr;
    }

    if (m_pCommandQueue)
    {
        m_pCommandQueue->Release();
        m_pCommandQueue = nullptr;
    }

    CleanupFence();
    CleanupDescriptorHeap();
    CleanupBuffers();

    if (m_pMaterialManager)
    {
        delete m_pMaterialManager;
        m_pMaterialManager = nullptr;
    }

    if (m_pTextureManager)
    {
        delete m_pTextureManager;
        m_pTextureManager = nullptr;
    }

    if (m_pFontManager)
    {
        delete m_pFontManager;
        m_pFontManager = nullptr;
    }

    if (m_pResourceManager)
    {
        delete m_pResourceManager;
        m_pResourceManager = nullptr;
    }

    if (m_pD3DDevice)
    {
        ULONG ref_count = m_pD3DDevice->Release();
        if (ref_count)
        {
            // resource leak!!!
            IDXGIDebug1 *pDebug = nullptr;
            if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
            {
                pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
                pDebug->Release();
            }
            __debugbreak();
        }

        m_pD3DDevice = nullptr;
    }
}

BOOL D3D12Renderer::InitRenderThreadPool(UINT threadCount)
{
    m_pThreadDescList = new RENDER_THREAD_DESC[threadCount];
    memset(m_pThreadDescList, 0, sizeof(RENDER_THREAD_DESC) * threadCount);

    m_hCompleteEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    for (UINT i = 0; i < threadCount; i++)
    {
        for (UINT j = 0; j < RENDER_THREAD_EVENT_TYPE_COUNT; j++)
        {
            m_pThreadDescList[i].hEventList[j] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        }
        m_pThreadDescList[i].pRenderer = this;
        m_pThreadDescList[i].threadindex = i;
        UINT threadID = 0;
        m_pThreadDescList[i].hThread =
            (HANDLE)_beginthreadex(nullptr, 0, RenderThread, m_pThreadDescList + i, 0, &threadID);
    }
    return TRUE;
}

void D3D12Renderer::CleanupRenderThreadPool()
{
    if (m_pThreadDescList)
    {
        for (UINT i = 0; i < m_renderThreadCount; i++)
        {
            SetEvent(m_pThreadDescList[i].hEventList[RENDER_THREAD_EVENT_TYPE_DESTROY]);

            WaitForSingleObject(m_pThreadDescList[i].hThread, INFINITE);
            CloseHandle(m_pThreadDescList[i].hThread);
            m_pThreadDescList[i].hThread = nullptr;

            for (UINT j = 0; j < RENDER_THREAD_EVENT_TYPE_COUNT; j++)
            {
                CloseHandle(m_pThreadDescList[i].hEventList[j]);
                m_pThreadDescList[i].hEventList[j] = nullptr;
            }
        }

        delete[] m_pThreadDescList;
        m_pThreadDescList = nullptr;
    }
    if (m_hCompleteEvent)
    {
        CloseHandle(m_hCompleteEvent);
        m_hCompleteEvent = nullptr;
    }
}
