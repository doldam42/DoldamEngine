#pragma once

#include "ConstantBuffers.h"
#include "RendererInterface.h"
#include "RenderQueue.h"
#include "RendererTypedef.h"

const UINT SWAP_CHAIN_FRAME_COUNT = 3;
const UINT MAX_PENDING_FRAME_COUNT = SWAP_CHAIN_FRAME_COUNT - 1;

struct CB_CONTAINER;
struct RENDER_THREAD_DESC;

class DescriptorPool;
class ConstantBufferPool;
class ConstantBufferManager;
class D3D12ResourceManager;
class FontManager;
class TextureManager;
class MaterialManager;
class CommandListPool;
class Cubemap;
class DXRSceneManager;

// Raytracing은 ID3D12GraphicsCommandList4 부터 사용가능
// #define USE_RAYTRACING
#define USE_MULTI_THREAD
// #define USE_MULTILPE_COMMAND_LIST

class D3D12Renderer : public IRenderer
{
    enum GLOBAL_DESCRIPTOR_INDEX
    {
        GLOBAL_DESCRIPTOR_INDEX_CB = 0,
        GLOBAL_DESCRIPTOR_INDEX_MATERIALS,
        GLOBAL_DESCRIPTOR_INDEX_IBL_ENV,
        GLOBAL_DESCRIPTOR_INDEX_IBL_SPECULAR,
        GLOBAL_DESCRIPTOR_INDEX_IBL_IRRADIANCE,
        GLOBAL_DESCRIPTOR_INDEX_IBL_BRDF,
        GLOBAL_DESCRIPTOR_INDEX_COUNT
    };

    static const UINT MAX_DRAW_COUNT_PER_FRAME = 4096;
    static const UINT MAX_DESCRIPTOR_COUNT = 4096;
    static const UINT MAX_RENDER_THREAD_COUNT = 8;

    HWND                  m_hWnd = nullptr;
    ID3D12Device5        *m_pD3DDevice = nullptr;
    ID3D12CommandQueue   *m_pCommandQueue = nullptr;
    D3D12ResourceManager *m_pResourceManager = nullptr;
    TextureManager       *m_pTextureManager = nullptr;
    MaterialManager      *m_pMaterialManager = nullptr;
    FontManager          *m_pFontManager = nullptr;

    // #DXR
    DXRSceneManager *m_pDXRSceneManager = nullptr;

    /* ID3D12CommandAllocator    *m_ppCommandAllocator[MAX_PENDING_FRAME_COUNT] = {};
     ID3D12GraphicsCommandList *m_ppCommandList[MAX_PENDING_FRAME_COUNT] = {};*/
    CommandListPool       *m_ppCommandListPool[MAX_PENDING_FRAME_COUNT][MAX_RENDER_THREAD_COUNT] = {};
    DescriptorPool        *m_ppDescriptorPool[MAX_PENDING_FRAME_COUNT][MAX_RENDER_THREAD_COUNT] = {};
    ConstantBufferManager *m_ppConstantBufferManager[MAX_PENDING_FRAME_COUNT][MAX_RENDER_THREAD_COUNT] = {};
    RenderQueue           *m_ppRenderQueue[MAX_RENDER_THREAD_COUNT] = {};
    RenderQueue           *m_pNonOpaqueRenderQueue = nullptr;

    UINT m_renderThreadCount = 0;
    UINT m_curThreadIndex = 0;

    FLOAT m_clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

    LONG volatile m_activeThreadCount = 0;
    HANDLE              m_hCompleteEvent = nullptr;
    RENDER_THREAD_DESC *m_pThreadDescList = nullptr;

    UINT64 m_pui64LastFenceValue[MAX_PENDING_FRAME_COUNT] = {};
    UINT64 m_ui64FenceValue = 0;

    D3D_FEATURE_LEVEL  m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;
    DXGI_ADAPTER_DESC1 m_AdapterDesc = {};
    IDXGISwapChain3   *m_pSwapChain = nullptr;
    D3D12_VIEWPORT     m_Viewport = {};
    D3D12_RECT         m_ScissorRect = {};
    float              m_DPI = 96.0f;

    ID3D12Resource *m_pRTOutputBuffers[SWAP_CHAIN_FRAME_COUNT] = {nullptr};
    ID3D12Resource *m_pRenderTargets[SWAP_CHAIN_FRAME_COUNT] = {nullptr};
    ID3D12Resource *m_pIntermediateRenderTargets[SWAP_CHAIN_FRAME_COUNT] = {nullptr};
    ID3D12Resource *m_pDepthStencil = nullptr;

    ID3D12DescriptorHeap *m_pRTVHeap = nullptr;
    ID3D12DescriptorHeap *m_pSRVHeap = nullptr;
    ID3D12DescriptorHeap *m_pDSVHeap = nullptr;
    ID3D12DescriptorHeap *m_pRaytracingOutputHeap = nullptr;

    // Global
    CB_CONTAINER   *m_pGlobalCB = nullptr;
    GlobalConstants m_globalConsts = {};
    Light           m_pLights[MAX_LIGHTS];

    TEXTURE_HANDLE *m_pDefaultTexHandle = nullptr;

    // CubeMap
    Cubemap *m_pCubemap = nullptr;

    UINT         m_srvDescriptorSize = 0;
    UINT         m_rtvDescriptorSize = 0;
    UINT         m_dwSwapChainFlags = 0;
    UINT         m_uiFrameIndex = 0;
    HANDLE       m_hFenceEvent = nullptr;
    ID3D12Fence *m_pFence = nullptr;

    UINT m_dwCurContextIndex = 0;

    void CreateDefaultTex();

    void CreateFence();
    void CleanupFence();
    BOOL CreateDescriptorHeap();
    void CleanupDescriptorHeap();

    UINT64 Fence();
    void   WaitForFenceValue(UINT64 ExpectedFenceValue);

    void CreateBuffers();
    void CleanupBuffers();
    void Cleanup();

    // For multi-threads
    BOOL InitRenderThreadPool(UINT threadCount);
    void CleanupRenderThreadPool();

  public:
    // Inherited via IRenderer
    BOOL Initialize(HWND hWnd, BOOL bEnableDebugLayer, BOOL bEnableGBV) override;
    void BeginRender() override;
    void EndRender() override;
    void Present() override;
    void OnUpdateWindowSize(UINT width, UINT height) override;

    IRenderMesh *CreateSkinnedObject() override;
    IRenderMesh *CreateMeshObject() override;

    IRenderSprite *CreateSpriteObject() override;
    IRenderSprite *CreateSpriteObject(const WCHAR *texFileName, int PosX, int PosY, int Width, int Height) override;

    void RenderMeshObject(IRenderMesh *pMeshObj, const Matrix *pWorldMat, bool isWired = false,
                          UINT numInstance = 1) override;
    void RenderCharacterObject(IRenderMesh *pCharObj, const Matrix *pWorldMat, const Matrix *pBoneMats, UINT numBones,
                               bool isWired = false) override;
    void RenderSpriteWithTex(IRenderSprite *pSprObjHandle, int iPosX, int iPosY, float fScaleX, float fScaleY,
                             const RECT *pRect, float Z, void *pTexHandle) override;
    void RenderSprite(IRenderSprite *pSprObjHandle, int iPosX, int iPosY, float fScaleX, float fScaleY,
                      float Z) override;

    IFontHandle *CreateFontObject(const WCHAR *fontFamilyName, float fontSize) override;
    void         DeleteFontObject(IFontHandle *pFontHandle) override;
    BOOL         WriteTextToBitmap(BYTE *pDestImage, UINT destWidth, UINT destHeight, UINT destPitch, int *pOutWidth,
                                   int *pOutHeight, IFontHandle *pFontObjHandle, const WCHAR *inStr, UINT len) override;

    BOOL BeginCreateMesh(IRenderMesh *pMeshObjHandle, const void *pVertices, UINT numVertices, UINT numFaceGroup,
                         const wchar_t *path) override;
    BOOL InsertFaceGroup(IRenderMesh *pMeshObjHandle, const UINT *pIndices, UINT numTriangles,
                         const Material *pInMaterial) override;
    void EndCreateMesh(IRenderMesh *pMeshObjHandle) override;

    void UpdateCamera(const Vector3 &eyeWorld, const Matrix &viewRow, const Matrix &projRow);
    void UpdateTextureWithImage(ITextureHandle *pTexHandle, const BYTE *pSrcBits, UINT srcWidth,
                                UINT srcHeight) override;

    ITextureHandle *CreateTiledTexture(UINT texWidth, UINT texHeight, UINT r, UINT g, UINT b);
    ITextureHandle *CreateDynamicTexture(UINT texWidth, UINT texHeight);
    ITextureHandle *CreateTextureFromFile(const WCHAR *fileName);
    ITextureHandle *CreateCubemapFromFile(const WCHAR *fileName);
    ITextureHandle *CreateMetallicRoughnessTexture(const WCHAR *metallicFilename, const WCHAR *roughneessFilename);
    void            DeleteTexture(ITextureHandle *pTexHandle);

    void *CreateDirectionalLight(const Vector3 *pRadiance, const Vector3 *pDirection);
    void *CreatePointLight(const Vector3 *pRadiance, const Vector3 *pDirection, const Vector3 *pPosition, float radius,
                           float fallOffStart = 0.0f, float fallOffEnd = 20.0f);
    void *CreateSpotLight(const Vector3 *pRadiance, const Vector3 *pDirection, const Vector3 *pPosition,
                          float spotPower, float radius, float fallOffStart = 0.0f, float fallOffEnd = 20.0f);
    void  DeleteLight(void *pLightHandle);

    IMaterialHandle *CreateMaterialHandle(const Material *pInMaterial) override;
    void             DeleteMaterialHandle(IMaterialHandle *pInMaterial) override;
    void             UpdateMaterialHandle(IMaterialHandle *pInMaterial, const Material *pMaterial) override;

    void InitCubemaps(const WCHAR *envFilename, const WCHAR *specularFilename, const WCHAR *irradianceFilename,
                      const WCHAR *brdfFilename) override;

    const Cubemap *GetCubemap() { return m_pCubemap; }

    UINT GetCommandListCount();

    float GetDPI() const { return m_DPI; }

    float INL_GetScreenWidth() const { return m_Viewport.Width; }
    float INL_GetScreenHeight() const { return m_Viewport.Height; }
    float GetAspectRatio() const { return float(m_Viewport.Width) / m_Viewport.Height; }

    // 함수 호출시 TEXTURE_HANDLE의 REF_COUNT를 1 올림
    TEXTURE_HANDLE *GetDefaultTex();

    CB_CONTAINER *INL_GetGlobalCB() { return m_pGlobalCB; }

    D3D12_GPU_DESCRIPTOR_HANDLE GetGlobalDescriptorHandle(UINT threadIndex);

    DescriptorPool *INL_GetDescriptorPool(UINT threadIndex)
    {
        return m_ppDescriptorPool[m_dwCurContextIndex][threadIndex];
    }

    ConstantBufferPool *GetConstantBufferPool(CONSTANT_BUFFER_TYPE type, UINT threadIndex);

    UINT GetSRVDescriptorSize() const { return m_srvDescriptorSize; }

    ID3D12Device5        *INL_GetD3DDevice() const { return m_pD3DDevice; }
    D3D12ResourceManager *INL_GetResourceManager() const { return m_pResourceManager; }
    DXRSceneManager      *INL_GetDXRSceneManager() const { return m_pDXRSceneManager; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHandle(RENDER_TARGET_TYPE type) const;

    // for RenderThread
    UINT GetRenderThreadCount() { return m_renderThreadCount; }
    void ProcessByThread(UINT threadIndex);

    void WaitForGPU();

    D3D12Renderer() = default;
    ~D3D12Renderer();
};
