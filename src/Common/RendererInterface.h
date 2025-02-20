#pragma once

#ifdef RENDERERMODULE_EXPORTS
#define RENDERERMODULE_API __declspec(dllexport)
#else
#define RENDERERMODULE_API __declspec(dllimport)
#endif

#include "../MathModule/MathHeaders.h"
#include <combaseapi.h>

enum TEXTURE_TYPE
{
    TEXTURE_TYPE_ALBEDO = 0,
    TEXTURE_TYPE_NORMAL,
    TEXTURE_TYPE_AO,
    TEXTURE_TYPE_EMISSIVE,
    TEXTURE_TYPE_METALLIC_ROUGHNESS,
    TEXTURE_TYPE_HEIGHT,
    TEXTURE_TYPE_COUNT
};

interface IFontHandle{};
interface ITextureHandle{};
interface ILightHandle{};

struct RENDER_GUI_PAYLOAD
{
    void *data;
    int   dataSize;
};

typedef int GUI_WINDOW_FLAGS;
enum GUI_WINDOW_FLAGS_ : UINT
{
    GUI_WINDOW_FLAG_NONE = 0,
    GUI_WINDOW_FLAG_NO_TITLE_BAR = 1 << 0,
    GUI_WINDOW_FLAG_NO_RESIZE = 1 << 1,
    GUI_WINDOW_FLAG_NO_MOVE = 1 << 2,
    GUI_WINDOW_FLAG_NO_SCROLLBAR = 1 << 3,
    GUI_WINDOW_FLAG_NO_SCROLL_WITH_MOUSE = 1 << 4,
    GUI_WINDOW_FLAG_NO_COLLAPSE = 1 << 5,
    GUI_WINDOW_FLAG_MENU_BAR = 1 << 10,
};

// v mean ui variable
interface IRenderGUI : public IUnknown
{
    virtual void DockSpace(const char *label) = 0;

    virtual BOOL Begin(const char *name, bool *pOpen = nullptr, GUI_WINDOW_FLAGS flags = GUI_WINDOW_FLAG_NONE) = 0;
    virtual void End() = 0;

    virtual BOOL BeginMenuBar() = 0;
    virtual void EndMenuBar() = 0;

    virtual BOOL BeginMenu(const char *label) = 0;
    virtual void EndMenu() = 0;
    virtual BOOL MenuItem(const char *label, const char *shortcut) = 0;

    virtual BOOL BeginChild(const char *label, float width = 0.0f, float height = 0.0f) = 0;
    virtual void EndChild() = 0;

    virtual void BeginGroup() = 0;
    virtual void EndGroup() = 0;

    virtual BOOL BeginTabBar(const char* label) = 0;
    virtual void EndTabBar() = 0;

    virtual BOOL BeginTabItem(const char *label) = 0;
    virtual void EndTabItem() = 0;

    // Drag and Drop
    virtual BOOL BeginDragDropSource() = 0;
    virtual void EndDragDropSource() = 0;
    virtual BOOL BeginDragDropTarget() = 0;
    virtual void EndDragDropTarget() = 0;
    virtual void SetDragDropPayload(const char *type, const void *data, size_t dataSize) = 0;
    virtual BOOL AcceptDragDropPayload(const char *type, RENDER_GUI_PAYLOAD* pOutPayload) = 0;

    virtual BOOL TreeNode(const char *name) = 0;
    virtual void TreePop() = 0;

    virtual void SameLine() = 0;

    virtual void Text(const char *fmt) = 0;
    virtual void CheckBox(const char *label, bool *v) = 0;
    virtual BOOL DragFloat3(const char *label, Vector3 *v, float delta = 0.1f) = 0;
    virtual void SliderFloat(const char *label, float *v, float vMin, float vMax, const char *fmt = "%.3f") = 0;
    virtual BOOL Button(const char *label) = 0;
    virtual BOOL ColoredButton(const char *label, RGBA color) = 0;
    virtual BOOL InvisibleButton() = 0;
    virtual void Image(ITextureHandle * pTexHanlde) = 0;

    // per pixel (Absolute)
    virtual void SetNextWindowPosA(UINT posX, UINT posY, bool onlyAtFirst = false) = 0;
    virtual void SetNextWindowSizeA(UINT width, UINT height, bool onlyAtFirst = false) = 0;

    // per percent (Relative)
    virtual void SetNextWindowPosR(float posX, float posY, bool onlyAtFirst = false) = 0;
    virtual void SetNextWindowSizeR(float width, float height, bool onlyAtFirst = false) = 0;

    // For ImGUI Win32 Event Handle
    virtual LRESULT WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) = 0;
};

interface IRenderMaterial : public IUnknown
{
    virtual BOOL UpdateMetallicRoughness(float metallic, float roughness) = 0;
    virtual BOOL UpdateTextureWithTexture(ITextureHandle * pTexture, TEXTURE_TYPE type) = 0;

    virtual ITextureHandle *GetTexture(TEXTURE_TYPE type) = 0;
};

interface IRenderableObject : public IUnknown{};
interface IRenderSprite : public IRenderableObject{};
interface IRenderTerrain : public IRenderableObject{};
interface IRenderMesh : public IRenderableObject
{
    virtual BOOL BeginCreateMesh(const void *pVertices, UINT numVertices, UINT numFaceGroup) = 0;
    virtual BOOL InsertFaceGroup(const UINT *pIndices, UINT numTriangles, const Material *pInMaterial,
                                 const wchar_t *path) = 0;
    virtual void EndCreateMesh() = 0;

    virtual BOOL UpdateMaterial(IRenderMaterial * pInMaterial, UINT faceGroupIndex) = 0;
};

interface IRenderer : public IUnknown
{
    virtual BOOL Initialize(HWND hWnd, BOOL bEnableDebugLayer, BOOL bEnableGBV) = 0;
    virtual void BeginRender() = 0;
    virtual void EndRender() = 0;
    virtual void Present() = 0;

    virtual void UpdateCamera(const Vector3 &eyeWorld, const Matrix &viewRow, const Matrix &projRow) = 0;

    virtual void OnUpdateWindowSize(UINT width, UINT height) = 0;

    virtual IRenderMesh *CreateSkinnedObject() = 0;
    virtual IRenderMesh *CreateMeshObject() = 0;

    virtual BOOL BeginCreateMesh(IRenderMesh * pMeshObjHandle, const void *pVertices, UINT numVertices,
                                 UINT numFaceGroup) = 0;
    virtual BOOL InsertFaceGroup(IRenderMesh * pMeshObjHandle, const UINT *pIndices, UINT numTriangles,
                                 const Material *pInMaterial, const wchar_t *path) = 0;
    virtual void EndCreateMesh(IRenderMesh * pMeshObjHandle) = 0;

    virtual IRenderSprite *CreateSpriteObject() = 0;
    virtual IRenderSprite *CreateSpriteObject(const WCHAR *texFileName, int PosX, int PosY, int Width, int Height) = 0;

    virtual IRenderTerrain *CreateTerrain(const Material *pMaterial, const Vector3 *pScale, const int numSlice = 1,
                                          const int numStack = 1) = 0;

    virtual void RenderMeshObject(IRenderMesh * pMeshObj, const Matrix *pWorldMat, bool isWired = false,
                                  UINT numInstance = 1) = 0;
    virtual void RenderMeshObjectWithMaterials(IRenderMesh * pMeshObj, const Matrix *pWorldMat,
                                               IRenderMaterial **ppMaterials, UINT numMaterial, bool isWired = false,
                                               UINT numInstance = 1) = 0;
    virtual void RenderCharacterObject(IRenderMesh * pCharObj, const Matrix *pWorldMat, const Matrix *pBoneMats,
                                       UINT numBones, bool isWired = false) = 0;
    virtual void RenderCharacterObjectWithMaterials(
        IRenderMesh * pCharObj, const Matrix *pWorldMat, const Matrix *pBoneMats, UINT numBones,
        IRenderMaterial **ppMaterials, UINT numMaterial, bool isWired = false) = 0;

    virtual void RenderSprite(IRenderSprite * pSprObjHandle, int iPosX, int iPosY, float fScaleX, float fScaleY,
                              float Z) = 0;
    virtual void RenderSpriteWithTex(IRenderSprite * pSprObjHandle, int iPosX, int iPosY, float fScaleX, float fScaleY,
                                     const RECT *pRect, float Z, ITextureHandle *pTexHandle) = 0;
    virtual void RenderTerrain(IRenderTerrain * pTerrain, const Vector3 *pScale, bool isWired = false) = 0;

    virtual IFontHandle *CreateFontObject(const WCHAR *fontFamilyName, float fontSize) = 0;
    virtual void         DeleteFontObject(IFontHandle * pFontHandle) = 0;
    virtual BOOL WriteTextToBitmap(BYTE * pDestImage, UINT destWidth, UINT destHeight, UINT destPitch, int *pOutWidth,
                                   int *pOutHeight, IFontHandle *pFontObjHandle, const WCHAR *inStr, UINT len) = 0;

    virtual void InitCubemaps(const WCHAR *envFilename, const WCHAR *specularFilename, const WCHAR *irradianceFilename,
                              const WCHAR *brdfFilename) = 0;

    virtual ITextureHandle *CreateTextureFromFile(const WCHAR *fileName) = 0;
    virtual ITextureHandle *CreateDynamicTexture(UINT texWidth, UINT texHeight) = 0;
    virtual void            DeleteTexture(ITextureHandle * pTexHandle) = 0;
    virtual void            UpdateTextureWithImage(ITextureHandle * pTexHandle, const BYTE *pSrcBits, UINT srcWidth,
                                                   UINT srcHeight) = 0;
    virtual void            UpdateTextureWithTexture(ITextureHandle * pDestTex, ITextureHandle * pSrcTex, UINT srcWidth,
                                                     UINT srcHeight) = 0;

    virtual IRenderMaterial *CreateMaterialHandle(const Material *pInMaterial = nullptr) = 0;
    virtual IRenderMaterial *CreateDynamicMaterial(const WCHAR *name) = 0;
    virtual void             DeleteMaterialHandle(IRenderMaterial * pInMaterial) = 0;
    virtual void             UpdateMaterialHandle(IRenderMaterial * pInMaterial, const Material *pMaterial) = 0;

    // return nullptr if lights are full
    virtual ILightHandle *CreateDirectionalLight(const Vector3 *pRadiance, const Vector3 *pDirection,
                                                 const Vector3 *pPosition, BOOL hasShadow = true) = 0;
    virtual ILightHandle *CreatePointLight(const Vector3 *pRadiance, const Vector3 *pDirection,
                                           const Vector3 *pPosition, float radius, float fallOffStart = 0.0f,
                                           float fallOffEnd = 20.0f, BOOL hasShadow = true) = 0;
    virtual ILightHandle *CreateSpotLight(const Vector3 *pRadiance, const Vector3 *pDirection, const Vector3 *pPosition,
                                          float spotPower, float radius, float fallOffStart = 0.0f,
                                          float fallOffEnd = 20.0f, BOOL hasShadow = true) = 0;
    virtual void          DeleteLight(ILightHandle * pLightHandle) = 0;

    // 텍스쳐 투영
    virtual void SetProjectionTexture(ITextureHandle * pTex) = 0;
    virtual void SetProjectionTextureViewProj(const Matrix *pViewRow, const Matrix *pProjRow) = 0;

    // For Debugging
    // virtual void UpdateTextureWithShadowMap(ITextureHandle * pTexHandle, UINT lightIndex) = 0;
    virtual ITextureHandle *GetShadowMapTexture(UINT lightIndex) = 0;

    virtual IRenderGUI *GetRenderGUI() = 0;

    virtual float GetAspectRatio() const = 0;
    virtual float GetDPI() const = 0;
    virtual UINT  GetCommandListCount() = 0;
    virtual UINT  GetRenderThreadCount() = 0;
};
