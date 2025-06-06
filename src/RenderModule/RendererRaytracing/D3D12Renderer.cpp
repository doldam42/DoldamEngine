#include "pch.h"

#include "../GenericModule/ProcessInfo.h"
#include "../GenericModule/StringUtil.h"
#include "CommandListPool.h"
#include "ConstantBufferManager.h"
#include "ConstantBufferPool.h"
#include "ConstantBuffers.h"
#include "D3D12ResourceManager.h"
#include "D3DUtil.h"
#include "DescriptorPool.h"
#include "FontManager.h"
#include "GraphicsCommon.h"
#include "MaterialManager.h"
#include "PrimitiveGenerator.h"
#include "RaytracingManager.h"
#include "RaytracingMeshObject.h"
#include "RenderThread.h"
#include "SpriteObject.h"
#include "Terrain.h"
#include "TextureManager.h"

#include "DebugLine.h"

#include "TransparencyManager.h"
#include "PostProcessor.h"

#include "GUIManager.h"
#include "SingleDescriptorAllocator.h"
// #include "PSOLibrary.h"

#include "D3D12Renderer.h"
#include <process.h>

D3D12Renderer *g_pRenderer = nullptr;

BOOL D3D12Renderer::Initialize(HWND hWnd, BOOL bEnableDebugLayer, BOOL bEnableGBV, BOOL enableTexOutput,
                               UINT viewportWidth, UINT viewportHeight)
{
    g_pRenderer = this;

    BOOL result = FALSE;
    m_DPI = GetDpiForWindow(hWnd);
    RECT rect;
    ::GetClientRect(hWnd, &rect);
    UINT wndWidth = rect.right - rect.left;
    UINT wndHeight = rect.bottom - rect.top;
    UINT backBufferWidth = rect.right - rect.left;
    UINT backBufferHeight = rect.bottom - rect.top;

    if (enableTexOutput)
    {
        m_Viewport.Width = (float)viewportWidth;
        m_Viewport.Height = (float)viewportHeight;
        m_Viewport.MinDepth = 0.0f;
        m_Viewport.MaxDepth = 1.0f;

        m_ScissorRect.left = 0;
        m_ScissorRect.top = 0;
        m_ScissorRect.right = viewportWidth;
        m_ScissorRect.bottom = viewportHeight;

        m_useTextureOutput = enableTexOutput;
    }
    else
    {
        m_Viewport.Width = (float)wndWidth;
        m_Viewport.Height = (float)wndHeight;
        m_Viewport.MinDepth = 0.0f;
        m_Viewport.MaxDepth = 1.0f;

        m_ScissorRect.left = 0;
        m_ScissorRect.top = 0;
        m_ScissorRect.right = wndWidth;
        m_ScissorRect.bottom = wndHeight;
    }

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
        swapChainDesc.Width = backBufferWidth;
        swapChainDesc.Height = backBufferHeight;
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

    m_pResourceManager = new D3D12ResourceManager;
    m_pResourceManager->Initialize(this, m_pCommandQueue, MAX_DESCRIPTOR_COUNT);

    CreateDescriptorTables();

    CreateBuffers();

    CreateFence();

    Graphics::InitCommonStates(m_pD3DDevice);

#ifdef USE_MULTI_THREAD
    DWORD physicalCoreCount = 0;
    DWORD logicalCoreCount = 0;
    TryGetPhysicalCoreCount(&physicalCoreCount, &logicalCoreCount);
    m_renderThreadCount = (UINT)physicalCoreCount;
    if (m_renderThreadCount > MAX_RENDER_THREAD_COUNT)
        m_renderThreadCount = MAX_RENDER_THREAD_COUNT;

    InitRenderThreadPool(m_renderThreadCount);
#else
    m_renderThreadCount = 1;
#endif
    for (UINT i = 0; i < m_renderThreadCount; i++)
    {
        m_ppRenderQueue[i] = new RenderQueue;
        m_ppRenderQueue[i]->Initialize(this, MAX_DRAW_COUNT_PER_FRAME);

        m_ppTranslucentRenderQueue[i] = new RenderQueue;
        m_ppTranslucentRenderQueue[i]->Initialize(this, MAX_DRAW_COUNT_PER_FRAME);
    }

    for (UINT i = 0; i < MAX_PENDING_FRAME_COUNT; i++)
    {
        for (UINT j = 0; j < m_renderThreadCount; j++)
        {
            m_ppCommandListPool[i][j] = new CommandListPool;
            m_ppCommandListPool[i][j]->Initialize(m_pD3DDevice, D3D12_COMMAND_LIST_TYPE_DIRECT, 2048);

            m_ppDescriptorPool[i][j] = new DescriptorPool;
            m_ppDescriptorPool[i][j]->Initialize(m_pD3DDevice, MAX_DRAW_COUNT_PER_FRAME);

            m_ppConstantBufferManager[i][j] = new ConstantBufferManager;
            m_ppConstantBufferManager[i][j]->Initialize(m_pD3DDevice, MAX_DRAW_COUNT_PER_FRAME);
        }
    }

    for (int i = 0; i < m_renderThreadCount; i++)
    {
        m_ppFrameBuffer[i] = new FrameBuffer;
        m_ppFrameBuffer[i]->Initialize(MAX_DRAW_COUNT_PER_FRAME * 1024);
    }

    m_pTextureManager = new TextureManager;
    m_pTextureManager->Initialize(this, 1024 / 16, 1024);

    m_pMaterialManager = new MaterialManager;
    m_pMaterialManager->Initialize(this, sizeof(MaterialConstants), 1024);

    m_pFontManager = new FontManager;
    m_pFontManager->Initialize(this, m_pCommandQueue, 1024, 256, bEnableDebugLayer);

    m_pPostProcessor = new PostProcessor;
    m_pPostProcessor->Initialize(this);

    m_pDebugLine = new DebugLine;
    m_pDebugLine->Initialize(this, 1024);

    m_pOITManager = new TransparencyManager;
    //m_pOITManager->Initialize(this, 4096 * 25 * 16, backBufferWidth, backBufferHeight);
    m_pOITManager->Initialize(this, 4096 * 25, backBufferWidth, backBufferHeight);

    m_pRaytracingManager = new RaytracingManager;

    CommandListPool            *pCommandListPool = GetCommandListPool(0);
    ID3D12GraphicsCommandList4 *pCommandList = pCommandListPool->GetCurrentCommandList();

    m_pRaytracingManager->Initialize(this, pCommandList, MAX_DRAW_COUNT_PER_FRAME, m_renderThreadCount);
    pCommandListPool->CloseAndExecute(m_pCommandQueue);

    m_pSingleDescriptorAllocator = new SingleDescriptorAllocator;
    m_pSingleDescriptorAllocator->Initialize(m_pD3DDevice, 32);

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    m_pSingleDescriptorAllocator->Alloc(&cpuHandle, &gpuHandle);

    m_pGUIManager = new GUIManager;
    m_pGUIManager->Initialize(m_hWnd, m_pD3DDevice, m_pCommandQueue, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT,
                              SWAP_CHAIN_FRAME_COUNT);

    CreateDefaultResources();

    m_pEnv = GetDefaultTex();

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
    // GUI Begin
    m_pGUIManager->BeginRender();

    CommandListPool           *pCommandListPool = GetCommandListPool(0);
    ID3D12GraphicsCommandList *pCommandList = pCommandListPool->GetCurrentCommandList();

    UpdateGlobal();
    m_pTextureManager->Update(pCommandList);
    m_pMaterialManager->Update(pCommandList);

#ifdef USE_DEFERRED_RENDERING
    CD3DX12_RESOURCE_BARRIER barriers[] = {
        CD3DX12_RESOURCE_BARRIER::Transition(m_pDiffuseRenderTargets[m_uiFrameIndex], D3D12_RESOURCE_STATE_COMMON,
                                             D3D12_RESOURCE_STATE_RENDER_TARGET),
        CD3DX12_RESOURCE_BARRIER::Transition(m_pNormalRenderTargets[m_uiFrameIndex], D3D12_RESOURCE_STATE_COMMON,
                                             D3D12_RESOURCE_STATE_RENDER_TARGET),
        CD3DX12_RESOURCE_BARRIER::Transition(m_pElementsRenderTargets[m_uiFrameIndex], D3D12_RESOURCE_STATE_COMMON,
                                             D3D12_RESOURCE_STATE_RENDER_TARGET),
        CD3DX12_RESOURCE_BARRIER::Transition(m_pDepthStencils[m_uiFrameIndex], D3D12_RESOURCE_STATE_COMMON,
                                             D3D12_RESOURCE_STATE_DEPTH_WRITE)};
    pCommandList->ResourceBarrier(_countof(barriers), barriers);

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = m_depthStencilDescriptorTables[m_uiFrameIndex].cpuHandle;
    pCommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    CD3DX12_CPU_DESCRIPTOR_HANDLE deferredRtvHandle(GetRTVHandle(RENDER_TARGET_TYPE_DEFERRED));
    pCommandList->ClearRenderTargetView(deferredRtvHandle, m_clearColor, 0, nullptr);
    deferredRtvHandle.Offset(m_rtvDescriptorSize);
    pCommandList->ClearRenderTargetView(deferredRtvHandle, m_clearColor, 0, nullptr);
    deferredRtvHandle.Offset(m_rtvDescriptorSize);
    pCommandList->ClearRenderTargetView(deferredRtvHandle, m_clearColor, 0, nullptr);
    deferredRtvHandle.Offset(m_rtvDescriptorSize);
#endif

    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets[m_uiFrameIndex],
                                                                           D3D12_RESOURCE_STATE_PRESENT,
                                                                           D3D12_RESOURCE_STATE_RENDER_TARGET));
    D3D12_CPU_DESCRIPTOR_HANDLE backBufferRTV = GetRTVHandle(RENDER_TARGET_TYPE_BACK);
    pCommandList->ClearRenderTargetView(backBufferRTV, m_clearColor, 0, nullptr);

    m_pOITManager->BeginRender(0, pCommandList);

    pCommandListPool->CloseAndExecute(m_pCommandQueue);
}

void D3D12Renderer::EndRender()
{
    CommandListPool            *pCommandListPool = GetCommandListPool(0);
    ID3D12GraphicsCommandList4 *pCommandList = pCommandListPool->GetCurrentCommandList();
    ID3D12Resource             *pOutputView = m_pRaytracingOutputBuffers[m_uiFrameIndex];
    D3D12_CPU_DESCRIPTOR_HANDLE uavHandle = GetUAVHandle(RENDER_TARGET_TYPE_RAYTRACING);

#if defined(USE_MULTI_THREAD) && defined(USE_DEFERRED_RENDERING)
    m_activeThreadCount = m_renderThreadCount;
    for (UINT i = 0; i < m_renderThreadCount; i++)
    {
        SetEvent(m_pThreadDescList[i].hEventList[RENDER_THREAD_EVENT_TYPE_PROCESS]);
    }
    WaitForSingleObject(m_hCompleteEvent, INFINITE);
#elif defined(USE_DEFERRED_RENDERING)
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(GetRTVHandle(RENDER_TARGET_TYPE_DEFERRED));
    D3D12_CPU_DESCRIPTOR_HANDLE   rtvs[] = {rtvHandle, rtvHandle.Offset(m_rtvDescriptorSize),
                                            rtvHandle.Offset(m_rtvDescriptorSize)};
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_depthStencilDescriptorTables[m_uiFrameIndex].cpuHandle);
    m_ppRenderQueue[0]->Process(0, pCommandListPool, m_pCommandQueue, 400, rtvs, &dsvHandle, &m_Viewport,
                                &m_ScissorRect, _countof(rtvs), DRAW_PASS_TYPE_DEFERRED);
#endif

    // DrawLine Debug
    {
        pCommandList = pCommandListPool->GetCurrentCommandList();
        pCommandList->RSSetViewports(1, &m_Viewport);
        pCommandList->RSSetScissorRects(1, &m_ScissorRect);
        pCommandList->OMSetRenderTargets(1, &GetRTVHandle(RENDER_TARGET_TYPE_DEFERRED), FALSE, &dsvHandle);
        m_pDebugLine->DrawLineAll(0, pCommandList);
        pCommandListPool->CloseAndExecute(m_pCommandQueue);
    }

    // Render Translucent Objects (OIT)
    {
        m_ppTranslucentRenderQueue[0]->Process(0, pCommandListPool, m_pCommandQueue, 400, nullptr, &dsvHandle,
                                               &m_Viewport, &m_ScissorRect, 0, DRAW_PASS_TYPE_TRANSPARENCY);
    }

    pCommandList = pCommandListPool->GetCurrentCommandList();

    m_pRaytracingManager->CreateTopLevelAS(pCommandList);

#ifdef USE_DEFERRED_RENDERING
    CD3DX12_RESOURCE_BARRIER barriers[] = {
        CD3DX12_RESOURCE_BARRIER::Transition(m_pDiffuseRenderTargets[m_uiFrameIndex],
                                             D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON),
        CD3DX12_RESOURCE_BARRIER::Transition(m_pNormalRenderTargets[m_uiFrameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET,
                                             D3D12_RESOURCE_STATE_COMMON),
        CD3DX12_RESOURCE_BARRIER::Transition(m_pElementsRenderTargets[m_uiFrameIndex],
                                             D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON),
        CD3DX12_RESOURCE_BARRIER::Transition(m_pDepthStencils[m_uiFrameIndex], D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                             D3D12_RESOURCE_STATE_COMMON)};
    pCommandList->ResourceBarrier(_countof(barriers), barriers);
    D3D12_CPU_DESCRIPTOR_HANDLE gbufferHandle = GetSRVHandle(RENDER_TARGET_TYPE_DEFERRED);

    m_pRaytracingManager->DispatchRay(0, pCommandList, pOutputView, uavHandle, gbufferHandle, 4);
#else
    m_pRaytracingManager->DispatchRay(0, pCommandList, pOutputView, uavHandle);
#endif

    D3D12_CPU_DESCRIPTOR_HANDLE backBufferRTV = GetRTVHandle(RENDER_TARGET_TYPE_BACK);
    if (m_useTextureOutput)
    {
        TEXTURE_HANDLE *pTexHandle = m_renderableTextureHanldes[m_uiFrameIndex];

        pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pTexHandle->pTexture,
                                                                               D3D12_RESOURCE_STATE_PRESENT,
                                                                               D3D12_RESOURCE_STATE_RENDER_TARGET));

        D3D12_CPU_DESCRIPTOR_HANDLE rtv = GetRTVHandle(RENDER_TARGET_TYPE_TEXTURE);
        D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = GetSRVHandle(RENDER_TARGET_TYPE_RAYTRACING);

        m_pPostProcessor->Draw(0, pCommandList, &m_Viewport, &m_ScissorRect, srvHandle, rtv);

        pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pTexHandle->pTexture,
                                                                               D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                                               D3D12_RESOURCE_STATE_PRESENT));
    }
    else
    {
        D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = GetSRVHandle(RENDER_TARGET_TYPE_RAYTRACING);
        m_pPostProcessor->Draw(0, pCommandList, &m_Viewport, &m_ScissorRect, srvHandle, backBufferRTV);
    }

    m_pOITManager->ResolveOIT(0, pCommandList, &m_Viewport, &m_ScissorRect, backBufferRTV);

    m_pGUIManager->EndRender(pCommandList, backBufferRTV);

    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets[m_uiFrameIndex],
                                                                           D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                                           D3D12_RESOURCE_STATE_PRESENT));

    pCommandListPool->CloseAndExecute(m_pCommandQueue);

    m_pRaytracingManager->Reset();
#ifdef USE_DEFERRED_RENDERING
    for (UINT i = 0; i < m_renderThreadCount; i++)
    {
        m_ppRenderQueue[i]->Reset();
        m_ppTranslucentRenderQueue[i]->Reset();
    }
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
    UINT nextContextIndex = (m_curContextIndex + 1) % MAX_PENDING_FRAME_COUNT;
    WaitForFenceValue(m_pui64LastFenceValue[nextContextIndex]);

    m_pOITManager->EndRender();

    for (UINT i = 0; i < m_renderThreadCount; i++)
    {
        // reset resources per frame
        m_ppConstantBufferManager[nextContextIndex][i]->Reset();
        m_ppDescriptorPool[nextContextIndex][i]->Reset();
        m_ppCommandListPool[nextContextIndex][i]->Reset();

        m_ppFrameBuffer[i]->Reset();
    }

    m_pResourceManager->UpdateManagedResource();

    m_curContextIndex = nextContextIndex;
}

void D3D12Renderer::OnUpdateWindowSize(UINT width, UINT height, UINT viewportWidth, UINT viewportHeigh)
{
    if (width == 0 || height == 0)
        return;

    WaitForGPU();

    if (m_useTextureOutput)
    {
        m_Viewport.Width = (float)viewportWidth;
        m_Viewport.Height = (float)viewportHeigh;

        m_ScissorRect.right = viewportWidth;
        m_ScissorRect.bottom = viewportHeigh;
    }
    else
    {
        m_Viewport.Width = (float)width;
        m_Viewport.Height = (float)height;

        m_ScissorRect.right = width;
        m_ScissorRect.bottom = height;
    }

    CleanupBuffers();

    if (FAILED(m_pSwapChain->ResizeBuffers(SWAP_CHAIN_FRAME_COUNT, width, height, DXGI_FORMAT_R8G8B8A8_UNORM,
                                           m_dwSwapChainFlags)))
    {
        __debugbreak();
    }

    m_uiFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

    CreateBuffers();

    m_pGUIManager->OnUpdateWindowSize(width, height);
}

IRenderMesh *D3D12Renderer::CreateSkinnedObject()
{
    IRenderMesh          *pObj = nullptr;
    RaytracingMeshObject *pMeshObj = new RaytracingMeshObject;

    pMeshObj->Initialize(this, RENDER_ITEM_TYPE_CHAR_OBJ);
    pMeshObj->AddRef();
    pObj = pMeshObj;

    return pMeshObj;
}

IRenderMesh *D3D12Renderer::CreateMeshObject()
{
    IRenderMesh          *pObj = nullptr;
    RaytracingMeshObject *pMeshObj = new RaytracingMeshObject;

    pMeshObj->Initialize(this, RENDER_ITEM_TYPE_MESH_OBJ);
    pMeshObj->AddRef();
    pObj = pMeshObj;

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

IRenderTerrain *D3D12Renderer::CreateTerrain(const Material *pMaterial, const Vector3 *pScale, const int numSlice,
                                             const int numStack)
{
    Terrain *pTerrain = new Terrain;
    pTerrain->Initialize(this, pScale, pMaterial, numSlice, numStack);
    pTerrain->AddRef();
    return pTerrain;
}

void D3D12Renderer::RenderMeshObject(IRenderMesh *pMeshObj, const Matrix *pWorldMat, IRenderMaterial **ppMaterials,
                                     UINT numMaterial, bool isWired, UINT numInstance)
{
    RaytracingMeshObject *pObj = (RaytracingMeshObject *)pMeshObj;

#ifdef USE_DEFERRED_RENDERING
    RENDER_ITEM item = {};
    item.pObjHandle = pObj;
    item.type = RENDER_ITEM_TYPE_MESH_OBJ;
    item.fillMode = isWired ? FILL_MODE_WIRED : FILL_MODE_SOLID;
    item.meshObjParam.worldTM = *pWorldMat;
    item.meshObjParam.ppKeyframes = nullptr;
    item.meshObjParam.frameCount = 0;
    item.meshObjParam.numMaterials = numMaterial;
    if (numMaterial > 0)
    {
        for (UINT i = 0; i < numMaterial; i++)
        {
            item.meshObjParam.ppMaterials[i] = ppMaterials[i];
        }
    }

    // Draw Translucent Object
    {
        BOOL isTranslucent = FALSE;
        for (int i = 0; i < numMaterial; i++)
        {
            MATERIAL_HANDLE *pMaterial = (MATERIAL_HANDLE *)ppMaterials[i];

            if (pMaterial->type == MATERIAL_TYPE_TRANSLUCENT)
            {
                isTranslucent = TRUE;
                break;
            }
        }
        if (isTranslucent && !m_ppTranslucentRenderQueue[GetCurrentThreadIndex()]->Add(&item))
            __debugbreak();

        if (isTranslucent)
        {
            return;
        }
    }

    if (!m_ppRenderQueue[GetCurrentThreadIndex()]->Add(&item))
        __debugbreak();

#elif defined(USE_FORWARD_RENDERING)
    CommandListPool            *pCommadListPool = m_ppCommandListPool[m_curContextIndex][0];
    ID3D12GraphicsCommandList4 *pCommandList = pCommadListPool->GetCurrentCommandList();
    pObj->Draw(0, pCommandList, pWorldMat, ppMaterials, numMaterial, nullptr, 0);
#endif
}

void D3D12Renderer::RenderCharacterObject(IRenderMesh *pCharObj, const Matrix *pWorldMat, IRenderMaterial **ppMaterials,
                                          UINT numMaterial, Keyframe **ppKeyframes, UINT frameCount, bool isWired)
{
#ifdef USE_DEFERRED_RENDERING
    RENDER_ITEM item = {};
    item.type = RENDER_ITEM_TYPE_CHAR_OBJ;
    item.pObjHandle = pCharObj;
    item.fillMode = isWired ? FILL_MODE_WIRED : FILL_MODE_SOLID;
    item.meshObjParam.worldTM = *pWorldMat;
    item.meshObjParam.ppKeyframes = ppKeyframes;
    item.meshObjParam.frameCount = frameCount;

    if (!ppMaterials)
    {
        item.meshObjParam.ppMaterials[0] = m_pDefaultMaterial;
        item.meshObjParam.numMaterials = 1;
    }
    else
    {
        for (UINT i = 0; i < numMaterial; i++)
        {
            item.meshObjParam.ppMaterials[i] = ppMaterials[i];
        }
        item.meshObjParam.numMaterials = numMaterial;
    }

    if (!m_ppRenderQueue[GetCurrentThreadIndex()]->Add(&item))
        __debugbreak();

    // Draw Translucent Object
    {
        BOOL isTranslucent = FALSE;
        for (int i = 0; i < numMaterial; i++)
        {
            MATERIAL_HANDLE *pMaterial = (MATERIAL_HANDLE *)ppMaterials[i];

            if (pMaterial->type == MATERIAL_TYPE_TRANSLUCENT)
            {
                isTranslucent = TRUE;
                break;
            }
        }
        if (isTranslucent && !m_ppTranslucentRenderQueue[GetCurrentThreadIndex()]->Add(&item))
            __debugbreak();
    }

#elif defined(USE_FORWARD_RENDERING)
    RaytracingMeshObject       *pObj = (RaytracingMeshObject *)pCharObj;
    CommandListPool            *pCommadListPool = m_ppCommandListPool[m_curContextIndex][0];
    ID3D12GraphicsCommandList4 *pCommandList = pCommadListPool->GetCurrentCommandList();
    pObj->Draw(0, pCommandList, pWorldMat, ppMaterials, numMaterial, pBoneMats, numBones);
#endif
}

void D3D12Renderer::RenderSpriteWithTex(IRenderSprite *pSprObjHandle, int iPosX, int iPosY, float fScaleX,
                                        float fScaleY, const RECT *pRect, float Z, ITextureHandle *pTexHandle)
{
#ifdef USE_DEFERRED_RENDERING
    RENDER_ITEM item = {};
    item.type = RENDER_ITEM_TYPE_SPRITE;
    item.pObjHandle = pSprObjHandle;
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

    if (!m_ppRenderQueue[GetCurrentThreadIndex()]->Add(&item))
        __debugbreak();
#endif
}

void D3D12Renderer::RenderSprite(IRenderSprite *pSprObjHandle, int iPosX, int iPosY, float fScaleX, float fScaleY,
                                 float Z)
{
#ifdef USE_DEFERRED_RENDERING
    RENDER_ITEM item = {};
    item.type = RENDER_ITEM_TYPE_SPRITE;
    item.pObjHandle = pSprObjHandle;
    item.spriteParam.posX = iPosX;
    item.spriteParam.posY = iPosY;
    item.spriteParam.scaleX = fScaleX;
    item.spriteParam.scaleY = fScaleY;
    item.spriteParam.isUseRect = FALSE;
    item.spriteParam.rect = {};
    item.spriteParam.pTexHandle = nullptr;
    item.spriteParam.Z = Z;

    if (!m_ppRenderQueue[GetCurrentThreadIndex()]->Add(&item))
        __debugbreak();
#endif
}

void D3D12Renderer::RenderTerrain(IRenderTerrain *pTerrain, const Vector3 *pScale, bool isWired)
{
#ifdef USE_DEFERRED_RENDERING
    Terrain *pObj = (Terrain *)pTerrain;

    RENDER_ITEM item = {};
    item.type = RENDER_ITEM_TYPE_TERRAIN;
    item.pObjHandle = pObj;
    item.terrainParam.scale = *pScale;
    item.terrainParam.fillMode = isWired ? FILL_MODE_WIRED : FILL_MODE_SOLID;

    if (!m_ppRenderQueue[GetCurrentThreadIndex()]->Add(&item))
        __debugbreak();
#endif
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
                                    const Joint *pJoint, UINT numJoint, UINT numFaceGroup)
{
    RaytracingMeshObject *pMeshObj = dynamic_cast<RaytracingMeshObject *>(pMeshObjHandle);
    BOOL                  result = pMeshObj->BeginCreateMesh(pVertices, numVertices, pJoint, numJoint, numFaceGroup);
    return result;
}

BOOL D3D12Renderer::InsertFaceGroup(IRenderMesh *pMeshObjHandle, const UINT *pIndices, UINT numTriangles,
                                    IRenderMaterial *pMaterial)
{
    RaytracingMeshObject *pMeshObj = reinterpret_cast<RaytracingMeshObject *>(pMeshObjHandle);
    BOOL                  result = pMeshObj->InsertFaceGroup(pIndices, numTriangles, pMaterial);

    return result;
}

void D3D12Renderer::EndCreateMesh(IRenderMesh *pMeshObjHandle)
{
    CommandListPool            *pCommandListPool = m_ppCommandListPool[m_curContextIndex][0];
    ID3D12GraphicsCommandList4 *pCommandList = pCommandListPool->GetCurrentCommandList();

    RaytracingMeshObject *pMeshObj = (RaytracingMeshObject *)pMeshObjHandle;
    pMeshObj->EndCreateMesh(pCommandList);
    pCommandListPool->CloseAndExecute(m_pCommandQueue);
}

void D3D12Renderer::UpdateCamera(const Vector3 &eyeWorld, const Matrix &viewRow, const Matrix &projRow)
{
    m_camPosition = eyeWorld;
    m_camViewRow = viewRow;
    m_camProjRow = projRow;
}

void D3D12Renderer::UpdateTextureWithImage(ITextureHandle *pTexHandle, const BYTE *pSrcBits, UINT srcWidth,
                                           UINT srcHeight)
{
    m_pTextureManager->UpdateTextureWithImage((TEXTURE_HANDLE *)pTexHandle, pSrcBits, srcWidth, srcHeight);
}

void D3D12Renderer::UpdateTextureWithTexture(ITextureHandle *pDestTex, ITextureHandle *pSrcTex, UINT srcWidth,
                                             UINT srcHeight)
{
    m_pTextureManager->UpdateTextureWithTexture((TEXTURE_HANDLE *)pDestTex, (TEXTURE_HANDLE *)pSrcTex, srcWidth,
                                                srcHeight);
}

void D3D12Renderer::UpdateGlobal()
{
    UpdateGlobalConstants(m_camPosition, m_camViewRow, m_camProjRow);

    // | Global Constants | Materials | IBL Textures |
    for (UINT i = 0; i < m_renderThreadCount; i++)
    {
        DescriptorPool *pDescriptorPool = GetDescriptorPool(i);

        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
        pDescriptorPool->Alloc(&cpuHandle, &gpuHandle, GLOBAL_DESCRIPTOR_INDEX_COUNT);

        CD3DX12_CPU_DESCRIPTOR_HANDLE cbHandle(cpuHandle, GLOBAL_DESCRIPTOR_INDEX_GLOBALCB, m_srvDescriptorSize);
        CD3DX12_CPU_DESCRIPTOR_HANDLE matHandle(cpuHandle, GLOBAL_DESCRIPTOR_INDEX_MATERIALS, m_srvDescriptorSize);
        CD3DX12_CPU_DESCRIPTOR_HANDLE cubemapHandle(cpuHandle, GLOBAL_DESCRIPTOR_INDEX_DIFFUSE_IBL,
                                                    m_srvDescriptorSize);

        m_pD3DDevice->CopyDescriptorsSimple(1, cbHandle, m_pGlobalCB->CBVHandle,
                                            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        m_pD3DDevice->CopyDescriptorsSimple(1, matHandle, m_pMaterialManager->GetSRV(),
                                            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        m_pD3DDevice->CopyDescriptorsSimple(1, cubemapHandle, m_pEnv->srv.cpuHandle,
                                            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        m_globalGpuDescriptorHandle[i] = gpuHandle;
    }
}

void D3D12Renderer::UpdateGlobalConstants(const Vector3 &eyeWorld, const Matrix &viewRow, const Matrix &projRow)
{
    ConstantBufferPool *pCBPool = GetConstantBufferPool(CONSTANT_BUFFER_TYPE_GLOBAL, 0);
    m_pGlobalCB = pCBPool->Alloc();

    m_globalConsts.eyeWorld = eyeWorld;
    m_globalConsts.view = viewRow.Transpose();
    m_globalConsts.proj = projRow.Transpose();
    m_globalConsts.invView = viewRow.Invert().Transpose();
    m_globalConsts.invProj = projRow.Invert().Transpose();
    m_globalConsts.viewProj = (viewRow * projRow).Transpose();
    m_globalConsts.invViewProj = (viewRow * projRow).Invert().Transpose();
    m_globalConsts.strengthIBL = STRENGTH_IBL;

    memcpy(&m_globalConsts.lights, m_pLights, sizeof(Light) * MAX_LIGHTS);
    memcpy(m_pGlobalCB->pSystemMemAddr, &m_globalConsts, sizeof(GlobalConstants));
}

ITextureHandle *D3D12Renderer::CreateTiledTexture(UINT texWidth, UINT texHeight, UINT r, UINT g, UINT b, UINT tileSize)
{
    DXGI_FORMAT TexFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    BYTE *pImage = (BYTE *)malloc(texWidth * texHeight * 4);
    memset(pImage, 0, texWidth * texHeight * 4);

    BOOL isBlackTile = TRUE;

    for (UINT y = 0; y < texHeight; y++)
    {
        for (UINT x = 0; x < texWidth; x++)
        {
            RGBA *pDest = (RGBA *)(pImage + (x + y * texWidth) * 4);
            isBlackTile = ((x / tileSize + y / tileSize) % 2 == 0);

            *pDest = isBlackTile ? RGBA{(BYTE)r, (BYTE)g, (BYTE)b, 255} : RGBA{0, 0, 0, 255};
        }
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

ITextureHandle *D3D12Renderer::CreateMetallicRoughnessTexture(const WCHAR *metallicFilename,
                                                              const WCHAR *roughneessFilename)
{
    TEXTURE_HANDLE *pTexHandle =
        m_pTextureManager->CreateMetallicRoughnessTexture(metallicFilename, roughneessFilename);
    return pTexHandle;
}

void D3D12Renderer::DeleteTexture(ITextureHandle *pTexHandle)
{
    WaitForGPU();

    m_pTextureManager->DeleteTexture(static_cast<TEXTURE_HANDLE *>(pTexHandle));
}

ILightHandle *D3D12Renderer::CreateDirectionalLight(const Vector3 *pRadiance, const Vector3 *pDirection,
                                                    const Vector3 *pPosition, BOOL hasShadow)
{
    Light light;
    light.direction = *pDirection;
    light.position = *pPosition;
    light.radiance = *pRadiance;
    light.type = LIGHT_DIRECTIONAL;
    light.type |= (hasShadow) ? LIGHT_SHADOW : 0;

    Matrix viewRow = XMMatrixLookToLH(light.position, light.direction, Vector3::UnitY);

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

ILightHandle *D3D12Renderer::CreatePointLight(const Vector3 *pRadiance, const Vector3 *pDirection,
                                              const Vector3 *pPosition, float radius, float fallOffStart,
                                              float fallOffEnd, BOOL hasShadow)
{
    Light light;
    light.direction = *pDirection;
    light.radiance = *pRadiance;
    light.position = *pPosition;
    light.radius = radius;
    light.fallOffStart = fallOffStart;
    light.fallOffEnd = fallOffEnd;

    light.type = LIGHT_POINT;
    light.type |= (hasShadow) ? LIGHT_SHADOW : 0;

    Matrix viewRow = XMMatrixLookToLH(light.position, light.direction, Vector3::UnitY);

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

ILightHandle *D3D12Renderer::CreateSpotLight(const Vector3 *pRadiance, const Vector3 *pDirection,
                                             const Vector3 *pPosition, float spotPower, float radius,
                                             float fallOffStart, float fallOffEnd, BOOL hasShadow)
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
    light.type |= (hasShadow) ? LIGHT_SHADOW : 0;

    Matrix viewRow = XMMatrixLookToLH(light.position, light.direction, Vector3::UnitY);

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

void D3D12Renderer::DeleteLight(ILightHandle *pLightHandle)
{
    // Boundary Check
    void *lightAddr = reinterpret_cast<void *>(pLightHandle);
    if (lightAddr < m_pLights || lightAddr >= ((uint8_t *)m_pLights + sizeof(Light) * MAX_LIGHTS))
    {
        __debugbreak();
    }
    Light *pLight = static_cast<Light *>(pLightHandle);
    pLight->type = LIGHT_OFF;
}

IRenderMaterial *D3D12Renderer::CreateMaterialHandle(const Material *pInMaterial, MATERIAL_TYPE type)
{
    MATERIAL_HANDLE *pMatHandle = m_pMaterialManager->CreateMaterial(pInMaterial, type);
    return pMatHandle;
}

IRenderMaterial *D3D12Renderer::CreateDynamicMaterial(const WCHAR *name)
{
    Material mat;
    wcscpy_s(mat.name, name);
    MATERIAL_HANDLE *pMatHandle = m_pMaterialManager->CreateMaterial(&mat, MATERIAL_TYPE_DEFAULT);
    return pMatHandle;
}

void D3D12Renderer::DeleteMaterialHandle(IRenderMaterial *pInMaterial)
{
    MATERIAL_HANDLE *pHandle = static_cast<MATERIAL_HANDLE *>(pInMaterial);
    m_pMaterialManager->DeleteMaterial(pHandle);
}

void D3D12Renderer::UpdateMaterialHandle(IRenderMaterial *pInMaterial, const Material *pMaterial)
{
    m_pMaterialManager->UpdateMaterial(static_cast<MATERIAL_HANDLE *>(pInMaterial), pMaterial);
}

void D3D12Renderer::InitCubemaps(const WCHAR *envFilename, const WCHAR *specularFilename,
                                 const WCHAR *irradianceFilename, const WCHAR *brdfFilename)
{
    if (m_pEnv)
    {
        DeleteTexture(m_pEnv);
        m_pEnv = nullptr;
    }

    m_pEnv = m_pTextureManager->CreateTextureFromFile(envFilename, TRUE);
}

void D3D12Renderer::SetProjectionTexture(ITextureHandle *pTex) {}

void D3D12Renderer::SetProjectionTextureViewProj(const Matrix *pViewRow, const Matrix *pProjRow) {}

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

// TODO: Addref를 하면서 적절히 처리할 수 있는 방법 찾기
MATERIAL_HANDLE *D3D12Renderer::GetDefaultMaterial() { return m_pDefaultMaterial; }

D3D12_GPU_DESCRIPTOR_HANDLE D3D12Renderer::GetGlobalDescriptorHandle(UINT threadIndex)
{
    return m_globalGpuDescriptorHandle[threadIndex];
}

ConstantBufferPool *D3D12Renderer::GetConstantBufferPool(CONSTANT_BUFFER_TYPE type, UINT threadIndex)
{
    return m_ppConstantBufferManager[m_curContextIndex][threadIndex]->GetConstantBufferPool(type);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12Renderer::GetRTVHandle(RENDER_TARGET_TYPE type) const
{
    switch (type)
    {
    case RENDER_TARGET_TYPE_BACK:
        return m_backRTVDescriptorTables[m_uiFrameIndex].cpuHandle;
    case RENDER_TARGET_TYPE_INTERMEDIATE:
        return m_intermediateRTVDescriptorTables[m_uiFrameIndex].cpuHandle;
    case RENDER_TARGET_TYPE_DEFERRED:
        return m_deferredRTVDescriptorTables[m_uiFrameIndex].cpuHandle;
    case RENDER_TARGET_TYPE_TEXTURE:
        return m_renderableTextureRTVTables[m_uiFrameIndex].cpuHandle;
    default:
        __debugbreak();
        break;
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12Renderer::GetSRVHandle(RENDER_TARGET_TYPE type) const
{
    switch (type)
    {
    case RENDER_TARGET_TYPE_INTERMEDIATE:
        return m_intermediateSRVDescriptorTables[m_uiFrameIndex].cpuHandle;
    case RENDER_TARGET_TYPE_RAYTRACING:
        return m_raytracingSRVDescriptorTables[m_uiFrameIndex].cpuHandle;
    case RENDER_TARGET_TYPE_DEFERRED:
        return m_deferredSRVDescriptorTables[m_uiFrameIndex].cpuHandle;
    default:
        __debugbreak();
        break;
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12Renderer::GetUAVHandle(RENDER_TARGET_TYPE type) const
{
    switch (type)
    {
    case RENDER_TARGET_TYPE_RAYTRACING:
        return m_raytracingUAVDescriptorTables[m_uiFrameIndex].cpuHandle;
    default:
        __debugbreak();
        break;
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12Renderer::GetDeferredRTV() const
{
    return m_deferredRTVDescriptorTables[m_uiFrameIndex].cpuHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12Renderer::GetDeferredSRV() const
{
    return m_deferredSRVDescriptorTables[m_uiFrameIndex].cpuHandle;
}

void D3D12Renderer::ProcessByThread(UINT threadIndex, DRAW_PASS_TYPE passType)
{
    CommandListPool *pCommandListPool = GetCommandListPool(threadIndex);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(GetRTVHandle(RENDER_TARGET_TYPE_DEFERRED));
    D3D12_CPU_DESCRIPTOR_HANDLE   rtvs[] = {rtvHandle, rtvHandle.Offset(m_rtvDescriptorSize),
                                            rtvHandle.Offset(m_rtvDescriptorSize)};
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_depthStencilDescriptorTables[m_uiFrameIndex].cpuHandle);

    m_ppRenderQueue[threadIndex]->Process(threadIndex, pCommandListPool, m_pCommandQueue, 400, rtvs, &dsvHandle,
                                          &m_Viewport, &m_ScissorRect, _countof(rtvs), passType);

    LONG curCount = _InterlockedDecrement(&m_activeThreadCount);
    if (curCount == 0)
    {
        SetEvent(m_hCompleteEvent);
    }
}

void D3D12Renderer::WaitForGPU()
{
    Fence();
    for (UINT i = 0; i < MAX_PENDING_FRAME_COUNT; i++)
    {
        WaitForFenceValue(m_pui64LastFenceValue[i]);
    }
}

IRenderGUI *D3D12Renderer::GetRenderGUI()
{
    m_pGUIManager->AddRef();
    return m_pGUIManager;
}

void *D3D12Renderer::FrameAlloc(UINT threadIndex, UINT sizeInByte)
{
    return m_ppFrameBuffer[threadIndex]->Alloc(sizeInByte);
}

D3D12Renderer::~D3D12Renderer()
{
    Cleanup();
#ifdef _DEBUG
    _ASSERT(_CrtCheckMemory());
#endif
}

HRESULT __stdcall D3D12Renderer::QueryInterface(REFIID riid, void **ppvObject) { return E_NOTIMPL; }

ULONG __stdcall D3D12Renderer::AddRef(void)
{
    m_refCount++;
    return m_refCount;
}

ULONG __stdcall D3D12Renderer::Release(void)
{
    ULONG ref_count = --m_refCount;
    if (!m_refCount)
        delete this;

    return ref_count;
}

IRenderMesh *D3D12Renderer::CreateSquareMesh(const float scale) { return PrimitiveGenerator::MakeSquare(scale); }

IRenderMesh *D3D12Renderer::CreateSphereMesh(const float radius, const int numSlices, const int numStacks)
{
    return PrimitiveGenerator::MakeSphere(radius, numSlices, numStacks);
}

IRenderMesh *D3D12Renderer::CreateBoxMesh(const float scale) { return PrimitiveGenerator::MakeBox(scale); }

IRenderMesh *D3D12Renderer::CreateWireBoxMesh(const Vector3 center, const Vector3 extends)
{
    return PrimitiveGenerator::MakeWireBox(center, extends);
}

void D3D12Renderer::DrawLine(const Vector3 &start, const Vector3 &end, const RGBA &color)
{
    m_pDebugLine->DrawLine(start, end, color);
}

void D3D12Renderer::CreateDefaultResources()
{
    // Create the vertex buffer.
    // Define the geometry for a triangle.
    SimpleVertex Vertices[] = {
        {{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, {{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, {{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
    };
    if (FAILED(m_pResourceManager->CreateVertexBuffer(&m_pFullScreenQuadVertexBuffer, &m_fullScreenQuadVertexBufferView,
                                                      sizeof(SimpleVertex), (UINT)_countof(Vertices), Vertices)))
    {
        __debugbreak();
    }

    m_pDefaultTexHandle = (TEXTURE_HANDLE *)CreateTiledTexture(512, 512, 255, 255, 255, 16);

    Material defaultMaterial;
    m_pDefaultMaterial = m_pMaterialManager->CreateMaterial(&defaultMaterial, MATERIAL_TYPE_DEFAULT);
}

void D3D12Renderer::CleanupDefaultResources()
{
    if (m_pFullScreenQuadVertexBuffer)
    {
        m_pFullScreenQuadVertexBuffer->Release();
        m_pFullScreenQuadVertexBuffer = nullptr;
    }

    if (m_pDefaultTexHandle)
    {
        DeleteTexture(m_pDefaultTexHandle);
        m_pDefaultTexHandle = nullptr;
    }

    if (m_pDefaultMaterial)
    {
        m_pDefaultMaterial->Release();
        m_pDefaultMaterial = nullptr;
    }
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

BOOL D3D12Renderer::CreateDescriptorTables()
{
    HRESULT hr = S_OK;

    for (int i = 0; i < SWAP_CHAIN_FRAME_COUNT; i++)
    {
        // 렌더타겟용 디스크립터 테이블
        m_pResourceManager->AllocRTVDescriptorTable(&m_backRTVDescriptorTables[i]);

        m_pResourceManager->AllocRTVDescriptorTable(&m_intermediateRTVDescriptorTables[i]);

        m_pResourceManager->AllocDescriptorTable(&m_intermediateSRVDescriptorTables[i], 1);

        m_pResourceManager->AllocDSVDescriptorTable(&m_depthStencilDescriptorTables[i]);

        if (m_useTextureOutput)
        {
            m_pResourceManager->AllocRTVDescriptorTable(&m_renderableTextureRTVTables[i]);
        }

        // 레이트레이싱용 디스크립터 테이블
        m_pResourceManager->AllocDescriptorTable(&m_raytracingSRVDescriptorTables[i], 1);
        m_pResourceManager->AllocDescriptorTable(&m_raytracingUAVDescriptorTables[i], 1);

        // Deferred Rendering 용 디스크립터 테이블
        m_pResourceManager->AllocRTVDescriptorTable(&m_deferredRTVDescriptorTables[i], DEFERRED_RENDER_TARGET_COUNT);
        m_pResourceManager->AllocDescriptorTable(&m_deferredSRVDescriptorTables[i],
                                                 DEFERRED_RENDER_TARGET_COUNT + 1); // Render Target + Depth
    }

    // Descriptor Size 저장
    m_rtvDescriptorSize = m_pD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_srvDescriptorSize = m_pD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    return TRUE;
}

void D3D12Renderer::CleanupDescriptorTables()
{
    for (int i = 0; i < SWAP_CHAIN_FRAME_COUNT; i++)
    {
        // FORWARD RENDERING
        m_pResourceManager->DeallocRTVDescriptorTable(&m_intermediateRTVDescriptorTables[i]);
        m_pResourceManager->DeallocDescriptorTable(&m_intermediateSRVDescriptorTables[i]);

        // DEFERRED RENDERING
        m_pResourceManager->DeallocRTVDescriptorTable(&m_deferredRTVDescriptorTables[i]);
        m_pResourceManager->DeallocDescriptorTable(&m_deferredSRVDescriptorTables[i]);

        // RAYTRACING
        m_pResourceManager->DeallocDescriptorTable(&m_raytracingSRVDescriptorTables[i]);
        m_pResourceManager->DeallocDescriptorTable(&m_raytracingUAVDescriptorTables[i]);

        // DEPTH STENCIL
        m_pResourceManager->DeallocDSVDescriptorTable(&m_depthStencilDescriptorTables[i]);
    }
}

UINT64 D3D12Renderer::Fence()
{
    m_ui64FenceValue++;
    m_pCommandQueue->Signal(m_pFence, m_ui64FenceValue);
    m_pui64LastFenceValue[m_curContextIndex] = m_ui64FenceValue;
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

// DEFERRED RTV TABLE
// |     Diffuse     |     Normal     |     Elements     |
void D3D12Renderer::CreateDeferredBuffers()
{
    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    clearValue.Color[0] = m_clearColor[0];
    clearValue.Color[1] = m_clearColor[1];
    clearValue.Color[2] = m_clearColor[2];
    clearValue.Color[3] = m_clearColor[3];

    D3D12_SHADER_RESOURCE_VIEW_DESC depthSrvDesc = {};
    depthSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    depthSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    depthSrvDesc.Texture2D.MipLevels = 1;
    depthSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    for (UINT n = 0; n < SWAP_CHAIN_FRAME_COUNT; n++)
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_deferredRTVDescriptorTables[n].cpuHandle);
        CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_deferredSRVDescriptorTables[n].cpuHandle);

        if (FAILED(m_pD3DDevice->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16G16B16A16_FLOAT, m_Viewport.Width, m_Viewport.Height, 1, 1,
                                              1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
                D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(&m_pDiffuseRenderTargets[n]))))
        {
            __debugbreak();
        }
        m_pDiffuseRenderTargets[n]->SetName(L"DiffuseRenderTarget");
        m_pD3DDevice->CreateRenderTargetView(m_pDiffuseRenderTargets[n], nullptr, rtvHandle);
        m_pD3DDevice->CreateShaderResourceView(m_pDiffuseRenderTargets[n], nullptr, srvHandle);
        rtvHandle.Offset(1, m_rtvDescriptorSize);
        srvHandle.Offset(1, m_srvDescriptorSize);

        if (FAILED(m_pD3DDevice->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16G16B16A16_FLOAT, m_Viewport.Width, m_Viewport.Height, 1, 1,
                                              1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
                D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(&m_pNormalRenderTargets[n]))))
        {
            __debugbreak();
        }
        m_pNormalRenderTargets[n]->SetName(L"NormalRenderTarget");
        m_pD3DDevice->CreateRenderTargetView(m_pNormalRenderTargets[n], nullptr, rtvHandle);
        m_pD3DDevice->CreateShaderResourceView(m_pNormalRenderTargets[n], nullptr, srvHandle);
        rtvHandle.Offset(1, m_rtvDescriptorSize);
        srvHandle.Offset(1, m_srvDescriptorSize);

        if (FAILED(m_pD3DDevice->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16G16B16A16_FLOAT, m_Viewport.Width, m_Viewport.Height, 1, 1,
                                              1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
                D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(&m_pElementsRenderTargets[n]))))
        {
            __debugbreak();
        }
        m_pElementsRenderTargets[n]->SetName(L"ElementsRenderTarget");
        m_pD3DDevice->CreateRenderTargetView(m_pElementsRenderTargets[n], nullptr, rtvHandle);
        m_pD3DDevice->CreateShaderResourceView(m_pElementsRenderTargets[n], nullptr, srvHandle);
        rtvHandle.Offset(1, m_rtvDescriptorSize);
        srvHandle.Offset(1, m_srvDescriptorSize);

        m_pD3DDevice->CreateShaderResourceView(m_pDepthStencils[n], &depthSrvDesc, srvHandle);
    }
}

void D3D12Renderer::CleanupDeferredBuffers()
{
    for (UINT i = 0; i < SWAP_CHAIN_FRAME_COUNT; i++)
    {
        if (m_pDiffuseRenderTargets[i])
        {
            m_pDiffuseRenderTargets[i]->Release();
            m_pDiffuseRenderTargets[i] = nullptr;
        }

        if (m_pNormalRenderTargets[i])
        {
            m_pNormalRenderTargets[i]->Release();
            m_pNormalRenderTargets[i] = nullptr;
        }

        if (m_pElementsRenderTargets[i])
        {
            m_pElementsRenderTargets[i]->Release();
            m_pElementsRenderTargets[i] = nullptr;
        }
    }
}

void D3D12Renderer::RenderFirstPass(ID3D12GraphicsCommandList *pCommandList) {}

void D3D12Renderer::RenderSecondPass(ID3D12GraphicsCommandList *pCommandList)
{
    CD3DX12_RESOURCE_BARRIER barriers[] = {
        CD3DX12_RESOURCE_BARRIER::Transition(m_pDiffuseRenderTargets[m_uiFrameIndex],
                                             D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON),
        CD3DX12_RESOURCE_BARRIER::Transition(m_pNormalRenderTargets[m_uiFrameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET,
                                             D3D12_RESOURCE_STATE_COMMON),
        CD3DX12_RESOURCE_BARRIER::Transition(m_pElementsRenderTargets[m_uiFrameIndex],
                                             D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON),
        CD3DX12_RESOURCE_BARRIER::Transition(m_pDepthStencils[m_uiFrameIndex], D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                             D3D12_RESOURCE_STATE_COMMON)};
    pCommandList->ResourceBarrier(_countof(barriers), barriers);

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = GetRTVHandle(RENDER_TARGET_TYPE_INTERMEDIATE);
    DescriptorPool             *pDescriptorPool = GetDescriptorPool(0);
    ID3D12DescriptorHeap       *pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();

    D3D12_GPU_DESCRIPTOR_HANDLE globalGpuHandle = GetGlobalDescriptorHandle(0);

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;

    // | DiffuseTex | NormalTex | ElementsTex | Depth |
    pDescriptorPool->Alloc(&cpuHandle, &gpuHandle, 4);

    m_pD3DDevice->CopyDescriptorsSimple(4, cpuHandle, GetSRVHandle(RENDER_TARGET_TYPE_DEFERRED),
                                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    pCommandList->RSSetViewports(1, &m_Viewport);
    pCommandList->RSSetScissorRects(1, &m_ScissorRect);
    pCommandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
    pCommandList->SetGraphicsRootSignature(Graphics::secondPassRS);
    pCommandList->SetPipelineState(Graphics::secondPassPSO);

    pCommandList->SetDescriptorHeaps(1, &pDescriptorHeap);
    pCommandList->SetGraphicsRootDescriptorTable(0, globalGpuHandle);
    pCommandList->SetGraphicsRootDescriptorTable(1, gpuHandle);

    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCommandList->IASetVertexBuffers(0, 1, &m_fullScreenQuadVertexBufferView);
    pCommandList->DrawInstanced(6, 1, 0, 0);
}

// Create frame resources.
// Create a RTV for each frame.
void D3D12Renderer::CreateBuffers()
{
    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    clearValue.Color[0] = m_clearColor[0];
    clearValue.Color[1] = m_clearColor[1];
    clearValue.Color[2] = m_clearColor[2];
    clearValue.Color[3] = m_clearColor[3];

    // Create Back RTV
    for (UINT n = 0; n < SWAP_CHAIN_FRAME_COUNT; n++)
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_backRTVDescriptorTables[n].cpuHandle);
        m_pSwapChain->GetBuffer(n, IID_PPV_ARGS(&m_pRenderTargets[n]));
        m_pD3DDevice->CreateRenderTargetView(m_pRenderTargets[n], nullptr, rtvHandle);
    }

    // Create Intermediate RTV
    for (UINT n = 0; n < SWAP_CHAIN_FRAME_COUNT; n++)
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_intermediateRTVDescriptorTables[n].cpuHandle);
        CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_intermediateSRVDescriptorTables[n].cpuHandle);

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
    }

    // Create Raytracing OutputBuffer
    for (UINT n = 0; n < SWAP_CHAIN_FRAME_COUNT; n++)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE uavHandle = m_raytracingUAVDescriptorTables[n].cpuHandle;
        D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = m_raytracingSRVDescriptorTables[n].cpuHandle;

        ID3D12Resource *pOutBuffer = nullptr;
        if (FAILED(m_pD3DDevice->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16G16B16A16_FLOAT, m_Viewport.Width, m_Viewport.Height, 1, 1,
                                              1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
                D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pOutBuffer))))
        {
            __debugbreak();
        }

        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        m_pD3DDevice->CreateUnorderedAccessView(pOutBuffer, nullptr, &uavDesc, uavHandle);
        m_pD3DDevice->CreateShaderResourceView(pOutBuffer, nullptr, srvHandle);

        m_pRaytracingOutputBuffers[n] = pOutBuffer;
    }

    // Create DSV
    for (UINT n = 0; n < SWAP_CHAIN_FRAME_COUNT; n++)
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
                D3D12_RESOURCE_STATE_COMMON, &depthOptimizedClearValue, IID_PPV_ARGS(&m_pDepthStencils[n]))))
        {
            __debugbreak();
        }
        m_pDepthStencils[n]->SetName(L"DepthStencil");
        m_pD3DDevice->CreateDepthStencilView(m_pDepthStencils[n], &depthStencilDesc,
                                             m_depthStencilDescriptorTables[n].cpuHandle);
    }

    if (m_useTextureOutput)
    {
        for (UINT n = 0; n < SWAP_CHAIN_FRAME_COUNT; n++)
        {
            // Create Texture Output
            m_renderableTextureHanldes[n] = m_pTextureManager->CreateRenderableTexture(
                m_Viewport.Width, m_Viewport.Height, DXGI_FORMAT_R8G8B8A8_UNORM);

            m_pD3DDevice->CreateRenderTargetView(m_renderableTextureHanldes[n]->pTexture, nullptr,
                                                 m_renderableTextureRTVTables[n].cpuHandle);
        }
    }

    CreateDeferredBuffers();
}

void D3D12Renderer::CleanupBuffers()
{
    CleanupDeferredBuffers();

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

        if (m_pRaytracingOutputBuffers[i])
        {
            m_pRaytracingOutputBuffers[i]->Release();
            m_pRaytracingOutputBuffers[i] = nullptr;
        }

        if (m_pDepthStencils[i])
        {
            m_pDepthStencils[i]->Release();
            m_pDepthStencils[i] = nullptr;
        }

        if (m_renderableTextureHanldes[i])
        {
            m_pTextureManager->DeleteTexture(m_renderableTextureHanldes[i]);
            m_renderableTextureHanldes[i] = nullptr;
        }
    }
}

void D3D12Renderer::Cleanup()
{
    WaitForGPU();

    CleanupDefaultResources();

#ifdef USE_DEFERRED_RENDERING
    CleanupRenderThreadPool();
#endif

    if (m_pDebugLine)
    {
        delete m_pDebugLine;
        m_pDebugLine = nullptr;
    }

    if (m_pSingleDescriptorAllocator)
    {
        delete m_pSingleDescriptorAllocator;
        m_pSingleDescriptorAllocator = nullptr;
    }
    if (m_pGUIManager)
    {
        delete m_pGUIManager;
        m_pGUIManager = nullptr;
    }

    if (m_pEnv)
    {
        DeleteTexture(m_pEnv);
        m_pEnv = nullptr;
    }

    if (m_pRaytracingManager)
    {
        delete m_pRaytracingManager;
        m_pRaytracingManager = nullptr;
    }

    Graphics::DeleteCommonStates();

    for (UINT i = 0; i < m_renderThreadCount; i++)
    {
        if (m_ppRenderQueue[i])
        {
            delete m_ppRenderQueue[i];
            m_ppRenderQueue[i] = nullptr;
        }

        if (m_ppTranslucentRenderQueue[i])
        {
            delete m_ppTranslucentRenderQueue[i];
            m_ppTranslucentRenderQueue[i] = nullptr;
        }
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
    CleanupDescriptorTables();
    CleanupBuffers();
    
    if (m_pOITManager)
    {
        delete m_pOITManager;
        m_pOITManager = nullptr;
    }

    if (m_pPostProcessor)
    {
        delete m_pPostProcessor;
        m_pPostProcessor = nullptr;
    }

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
        m_pThreadDescList[i].passType = DRAW_PASS_TYPE_DEFERRED;
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
