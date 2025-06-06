#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>

#include "ConstantBuffers.h"
#include "RenderQueue.h"

#include "../MathModule/MathHeaders.h"
#include "../Common/RendererInterface.h"
#include "RendererTypedef.h"

const UINT  SWAP_CHAIN_FRAME_COUNT = 3;
const UINT  MAX_PENDING_FRAME_COUNT = SWAP_CHAIN_FRAME_COUNT - 1;
const float STRENGTH_IBL = 0.2f;

struct CB_CONTAINER;
struct RENDER_THREAD_DESC;

class ShadowManager;
class DescriptorPool;
class ConstantBufferPool;
class ConstantBufferManager;
class D3D12ResourceManager;
class FontManager;
class TextureManager;
class MaterialManager;
class CommandListPool;
class Cubemap;
class RaytracingManager;
class Terrain;

class PostProcessor;

//#define USE_FORWARD_RENDERING
#define USE_DEFERRED_RENDERING
enum GLOBAL_DESCRIPTOR_INDEX
{
    GLOBAL_DESCRIPTOR_INDEX_GLOBALCB = 0,
    GLOBAL_DESCRIPTOR_INDEX_MATERIALS,
    GLOBAL_DESCRIPTOR_INDEX_CUBE_MAP1,
    GLOBAL_DESCRIPTOR_INDEX_CUBE_MAP2,
    GLOBAL_DESCRIPTOR_INDEX_CUBE_MAP3,
    GLOBAL_DESCRIPTOR_INDEX_CUBE_MAP4,
    GLOBAL_DESCRIPTOR_INDEX_PROJECTION_TEX,
    GLOBAL_DESCRIPTOR_INDEX_SHADOW_MAP1,
    GLOBAL_DESCRIPTOR_INDEX_SHADOW_MAP2,
    GLOBAL_DESCRIPTOR_INDEX_SHADOW_MAP3,
    GLOBAL_DESCRIPTOR_INDEX_COUNT
};

class D3D12Renderer : public IRenderer
{
    enum DSV_DESCRIPTOR_INDEX
    {
        DSV_DESCRIPTOR_INDEX_COMMON = 0,
        DSV_DESCRIPTOR_INDEX_SHADOW,
    };

    static const UINT MAX_DRAW_COUNT_PER_FRAME = 4096;
    static const UINT MAX_DESCRIPTOR_COUNT = 4096;
    static const UINT MAX_RENDER_THREAD_COUNT = 8;

    static const UINT DEFERRED_RENDER_TARGET_COUNT = 3;

    BOOL m_useTextureOutput = FALSE;

    Camera *m_pMainCamera = nullptr;

    ULONG m_refCount = 1;

    HWND                  m_hWnd = nullptr;
    ID3D12Device5        *m_pD3DDevice = nullptr;
    ID3D12CommandQueue   *m_pCommandQueue = nullptr;
    D3D12ResourceManager *m_pResourceManager = nullptr;
    TextureManager       *m_pTextureManager = nullptr;
    MaterialManager      *m_pMaterialManager = nullptr;
    FontManager          *m_pFontManager = nullptr;

    PostProcessor *m_pPostProcessor = nullptr;

    CommandListPool       *m_ppCommandListPool[MAX_PENDING_FRAME_COUNT][MAX_RENDER_THREAD_COUNT] = {};
    DescriptorPool        *m_ppDescriptorPool[MAX_PENDING_FRAME_COUNT][MAX_RENDER_THREAD_COUNT] = {};
    ConstantBufferManager *m_ppConstantBufferManager[MAX_PENDING_FRAME_COUNT][MAX_RENDER_THREAD_COUNT] = {};
    RenderQueue           *m_ppRenderQueue[MAX_RENDER_THREAD_COUNT] = {};
    // RenderQueue           *m_pNonOpaqueRenderQueue = nullptr;

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

    ID3D12Resource *m_pRenderTargets[SWAP_CHAIN_FRAME_COUNT] = {nullptr};
    ID3D12Resource *m_pIntermediateRenderTargets[SWAP_CHAIN_FRAME_COUNT] = {nullptr};
    ID3D12Resource *m_pDepthStencils[SWAP_CHAIN_FRAME_COUNT] = {nullptr};

    // DEFERRED RenderTargets
    ID3D12Resource   *m_pDiffuseRenderTargets[SWAP_CHAIN_FRAME_COUNT] = {nullptr};
    ID3D12Resource   *m_pNormalRenderTargets[SWAP_CHAIN_FRAME_COUNT] = {nullptr};
    ID3D12Resource   *m_pElementsRenderTargets[SWAP_CHAIN_FRAME_COUNT] = {nullptr};
    DESCRIPTOR_HANDLE m_deferredRTVDescriptorTables[SWAP_CHAIN_FRAME_COUNT] = {};
    DESCRIPTOR_HANDLE m_deferredSRVDescriptorTables[SWAP_CHAIN_FRAME_COUNT] = {};

    // Shadow Map
    // CascadedShadowsManager *m_pCascadedShadowManager = nullptr;
    ShadowManager *m_pShadowManager = nullptr;
    UINT           m_shadowWidth = 1024;

    DESCRIPTOR_HANDLE m_intermediateRTVDescriptorTables[SWAP_CHAIN_FRAME_COUNT] = {};
    DESCRIPTOR_HANDLE m_intermediateSRVDescriptorTables[SWAP_CHAIN_FRAME_COUNT] = {};

    DESCRIPTOR_HANDLE m_backRTVDescriptorTables[SWAP_CHAIN_FRAME_COUNT] = {};

    DESCRIPTOR_HANDLE m_depthStencilDescriptorTables[SWAP_CHAIN_FRAME_COUNT] = {};
    
    TEXTURE_HANDLE   *m_renderableTextureHanldes[SWAP_CHAIN_FRAME_COUNT];
    DESCRIPTOR_HANDLE m_renderableTextureRTVTables[SWAP_CHAIN_FRAME_COUNT];

    // Global
    Matrix  m_camViewRow;
    Matrix  m_camProjRow;
    Vector3 m_camPosition;

    Matrix m_projectionViewProjRow;

    CB_CONTAINER               *m_pGlobalCB = nullptr;
    GlobalConstants             m_globalConsts = {};
    D3D12_GPU_DESCRIPTOR_HANDLE m_globalGpuDescriptorHandle[MAX_RENDER_THREAD_COUNT];

    Light m_pLights[MAX_LIGHTS];

    TEXTURE_HANDLE *m_pDefaultTexHandle = nullptr;
    TEXTURE_HANDLE *m_pProjectionTexHandle = nullptr;

    // CubeMap
    Cubemap *m_pCubemap = nullptr;

    UINT         m_dsvDescriptorSize = 0;
    UINT         m_srvDescriptorSize = 0;
    UINT         m_rtvDescriptorSize = 0;
    UINT         m_dwSwapChainFlags = 0;
    UINT         m_uiFrameIndex = 0;
    HANDLE       m_hFenceEvent = nullptr;
    ID3D12Fence *m_pFence = nullptr;

    UINT m_curContextIndex = 0;

  private:
    void CreateDefaultTex();

    void CreateFence();
    void CleanupFence();
    BOOL CreateDescriptorTables();
    void CleanupDescriptorTables();

    UINT64 Fence();
    void   WaitForFenceValue(UINT64 ExpectedFenceValue);

    void CreateBuffers();
    void CleanupBuffers();
    void Cleanup();

    // For multi-hThreads
    BOOL InitRenderThreadPool(UINT threadCount);
    void CleanupRenderThreadPool();

    void UpdateGlobal();
    void UpdateGlobalConstants(const Vector3 &eyeWorld, const Matrix &viewRow, const Matrix &projRow);

    // Deferred RenderTargets
    void CreateDeferredBuffers();
    void CleanupDeferredBuffers();

    void RenderSecondPass(ID3D12GraphicsCommandList *pCommandList);

  public:
    // Inherited via IRenderer
    BOOL Initialize(HWND hWnd, BOOL bEnableDebugLayer, BOOL bEnableGBV, BOOL enableTexOutput = FALSE,
                    UINT viewportWidth = 0, UINT viewportHeight = 0) override;
    void BeginRender() override;
    void EndRender() override;
    void Present() override;
    void OnUpdateWindowSize(UINT width, UINT height, UINT viewportWidth = 0, UINT viewportHeigh = 0) override;

    IRenderMesh *CreateSkinnedObject() override;
    IRenderMesh *CreateMeshObject() override;

    IRenderSprite *CreateSpriteObject() override;
    IRenderSprite *CreateSpriteObject(const WCHAR *texFileName, int PosX, int PosY, int Width, int Height) override;

    IRenderTerrain *CreateTerrain(const Material *pMaterial, const Vector3 *pScale, const int numSlice = 1,
                                  const int numStack = 1) override;

    void RenderMeshObject(IRenderMesh *pMeshObj, const Matrix *pWorldMat, IRenderMaterial **ppMaterials,
                          UINT numMaterial, bool isWired = false, UINT numInstance = 1) override;
    void RenderCharacterObject(IRenderMesh *pCharObj, const Matrix *pWorldMat, IRenderMaterial **ppMaterials,
                               UINT numMaterial, Keyframe **ppKeyframes, UINT frameCount,
                               bool isWired = false) override;
    void RenderSpriteWithTex(IRenderSprite *pSprObjHandle, int iPosX, int iPosY, float fScaleX, float fScaleY,
                             const RECT *pRect, float Z, ITextureHandle *pTexHandle) override;
    void RenderSprite(IRenderSprite *pSprObjHandle, int iPosX, int iPosY, float fScaleX, float fScaleY,
                      float Z) override;
    void RenderTerrain(IRenderTerrain *pTerrain, const Vector3 *pScale, bool isWired = false) override;

    IFontHandle *CreateFontObject(const WCHAR *fontFamilyName, float fontSize) override;
    void         DeleteFontObject(IFontHandle *pFontHandle) override;
    BOOL         WriteTextToBitmap(BYTE *pDestImage, UINT destWidth, UINT destHeight, UINT destPitch, int *pOutWidth,
                                   int *pOutHeight, IFontHandle *pFontObjHandle, const WCHAR *inStr, UINT len) override;

    BOOL BeginCreateMesh(IRenderMesh *pMeshObjHandle, const void *pVertices, UINT numVertices, const Joint *pJoint,
                         UINT numJoint, UINT numFaceGroup) override;
    BOOL InsertFaceGroup(IRenderMesh *pMeshObjHandle, const UINT *pIndices, UINT numTriangles) override;
    void EndCreateMesh(IRenderMesh *pMeshObjHandle) override;

    void UpdateCamera(const Vector3 &eyeWorld, const Matrix &viewRow, const Matrix &projRow);
    void UpdateTextureWithImage(ITextureHandle *pTexHandle, const BYTE *pSrcBits, UINT srcWidth,
                                UINT srcHeight) override;
    void UpdateTextureWithTexture(ITextureHandle *pDestTex, ITextureHandle *pSrcTex, UINT srcWidth,
                                  UINT srcHeight) override;

    ITextureHandle *CreateTiledTexture(UINT texWidth, UINT texHeight, UINT r, UINT g, UINT b);
    ITextureHandle *CreateDynamicTexture(UINT texWidth, UINT texHeight);
    ITextureHandle *CreateTextureFromFile(const WCHAR *fileName);
    ITextureHandle *CreateCubemapFromFile(const WCHAR *fileName);
    ITextureHandle *CreateMetallicRoughnessTexture(const WCHAR *metallicFilename, const WCHAR *roughneessFilename);
    void            DeleteTexture(ITextureHandle *pTexHandle);

    ILightHandle *CreateDirectionalLight(const Vector3 *pRadiance, const Vector3 *pDirection, const Vector3 *pPosition,
                                         BOOL hasShadow = true) override;
    ILightHandle *CreatePointLight(const Vector3 *pRadiance, const Vector3 *pDirection, const Vector3 *pPosition,
                                   float radius, float fallOffStart = 0.0f, float fallOffEnd = 20.0f,
                                   BOOL hasShadow = true) override;
    ILightHandle *CreateSpotLight(const Vector3 *pRadiance, const Vector3 *pDirection, const Vector3 *pPosition,
                                  float spotPower, float radius, float fallOffStart = 0.0f, float fallOffEnd = 20.0f,
                                  BOOL hasShadow = true) override;
    void          DeleteLight(ILightHandle *pLightHandle);

    IRenderMaterial *CreateMaterialHandle(const Material *pInMaterial, MATERIAL_TYPE type = MATERIAL_TYPE_DEFAULT) override;
    IRenderMaterial *CreateDynamicMaterial(const WCHAR *name) override;
    void             DeleteMaterialHandle(IRenderMaterial *pInMaterial) override;
    void             UpdateMaterialHandle(IRenderMaterial *pInMaterial, const Material *pMaterial) override;

    void InitCubemaps(const WCHAR *envFilename, const WCHAR *specularFilename, const WCHAR *irradianceFilename,
                      const WCHAR *brdfFilename) override;

    // Texture 투영
    void SetProjectionTexture(ITextureHandle *pTex) override;
    void SetProjectionTextureViewProj(const Matrix *pViewRow, const Matrix *pProjRow) override;

    const Cubemap *GetCubemap() { return m_pCubemap; }

    UINT GetCommandListCount();

    float GetDPI() const { return m_DPI; }

    float GetScreenWidth() const { return m_Viewport.Width; }
    float GetScreenHeight() const { return m_Viewport.Height; }
    float GetAspectRatio() const { return float(m_Viewport.Width) / m_Viewport.Height; }

    D3D12_VIEWPORT GetViewport() const { return m_Viewport; }
    D3D12_RECT     GetScissorRect() const { return m_ScissorRect; }

    // 함수 호출시 TEXTURE_HANDLE의 REF_COUNT를 1 올림

    TEXTURE_HANDLE *GetDefaultTex();

    CB_CONTAINER *INL_GetGlobalCB() { return m_pGlobalCB; }

    D3D12_GPU_DESCRIPTOR_HANDLE GetGlobalDescriptorHandle(UINT threadIndex);

    DescriptorPool *GetDescriptorPool(UINT threadIndex) const
    {
        return m_ppDescriptorPool[m_curContextIndex][threadIndex];
    }

    CommandListPool *GetCommandListPool(UINT threadIndex) const
    {
        return m_ppCommandListPool[m_curContextIndex][threadIndex];
    }

    ConstantBufferPool *GetConstantBufferPool(CONSTANT_BUFFER_TYPE type, UINT threadIndex);

    UINT GetSRVDescriptorSize() const { return m_srvDescriptorSize; }

    ID3D12Device5        *GetD3DDevice() const { return m_pD3DDevice; }
    D3D12ResourceManager *GetResourceManager() const { return m_pResourceManager; }
    TextureManager       *GetTextureManager() const { return m_pTextureManager; }
    MaterialManager      *GetMaterialManager() const { return m_pMaterialManager; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHandle(RENDER_TARGET_TYPE type) const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetSRVHandle(RENDER_TARGET_TYPE type) const;

    D3D12_CPU_DESCRIPTOR_HANDLE GetDeferredRTV() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetDeferredSRV() const;

    // void UpdateTextureWithShadowMap(ITextureHandle *pTexHandle, UINT lightIndex) override;
    ITextureHandle *GetShadowMapTexture(UINT lightIndex) override;

    ITextureHandle *GetRenderTargetTexture() override { return m_renderableTextureHanldes[m_curContextIndex]; }

    // for RenderThread
    UINT GetRenderThreadCount() { return m_renderThreadCount; }
    void ProcessByThread(UINT threadIndex, DRAW_PASS_TYPE passType);

    void WaitForGPU();

    IRenderGUI *GetRenderGUI() override { return nullptr; }

    D3D12Renderer() = default;
    ~D3D12Renderer();

    // Inherited via IRenderer
    HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject) override;
    ULONG __stdcall AddRef(void) override;
    ULONG __stdcall Release(void) override;

    // Inherited via IRenderer
    IRenderMesh *CreateSquareMesh(const float scale) override;
    IRenderMesh *CreateSphereMesh(const float radius, const int numSlices, const int numStacks) override;
    IRenderMesh *CreateBoxMesh(const float scale) override;
    IRenderMesh *CreateWireBoxMesh(const Vector3 center, const Vector3 extends) override;

    // Inherited via IRenderer
    void DrawLine(const Vector3 &start, const Vector3 &end, const RGBA &color) override;
};

extern D3D12Renderer *g_pRenderer;