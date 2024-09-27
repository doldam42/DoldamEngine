#pragma once

#ifdef ENGINEMODULE_EXPORTS
#define ENGINEMODULE_API __declspec(dllexport)
#else
#define ENGINEMODULE_API __declspec(dllimport)
#endif

#include <combaseapi.h>
#include "../RenderModule/RendererInterface.h"
#include "EngineTypedef.h"

enum PRIMITIVE_MODEL_TYPE
{
    PRIMITIVE_MODEL_TYPE_SQUARE = 0,
    PRIMITIVE_MODEL_TYPE_BOX,
};

interface IModel : public IUnknown
{
    virtual void Initialize(const Material *pInMaterial, int materialCount, void **ppInObjs, int objectCount) = 0;
    virtual void WriteFile(FILE * fp) = 0;
    virtual void ReadFile(FILE * fp) = 0;
};

interface IMeshObject
{
    virtual BOOL Initialize(MESH_TYPE meshType) = 0;
    virtual void BeginCreateMesh(const void *pVertices, UINT numVertices, UINT numFaceGroup) = 0;
    virtual void InsertFaceGroup(const UINT *pIndices, UINT numIndices, int materialIdx) = 0;
    virtual void EndCreateMesh() = 0;
};

//interface ICharacterObject
//{
//    virtual void BeginCreateMesh(const void *pVertices, uint32_t numVertices, uint32_t numFaceGroup) = 0;
//    virtual void InsertFaceGroup(const uint32_t *pIndices, uint32_t numIndices, int materialIdx) = 0;
//    virtual void EndCreateMesh() = 0;
//};

interface IGameObject
{
    virtual Vector3 GetPosition() = 0;
    virtual Vector3 GetScale() = 0;
    virtual float   GetRotationX() = 0;
    virtual float   GetRotationY() = 0;
    virtual float   GetRotationZ() = 0;

    virtual void SetModel(IModel * pModel) = 0;
    virtual void SetPosition(float x, float y, float z) = 0;
    virtual void SetScale(float x, float y, float z) = 0;
    virtual void SetScale(float s) = 0;
    virtual void SetRotationX(float rotX) = 0;
    virtual void SetRotationY(float rotY) = 0;
    virtual void SetRotationZ(float rotZ) = 0;
};

interface ISprite
{
    virtual BOOL UpdateTextureWidthImage(const BYTE *pSrcBits, UINT srcWidth, UINT srcHeight) = 0;

    // Getter
    virtual UINT GetWidth() = 0;
    virtual UINT GetHeight() = 0;

    virtual int GetPosX() = 0;
    virtual int GetPosY() = 0;

    virtual float GetScaleX() = 0;
    virtual float GetScaleY() = 0;

    virtual float GetZ() = 0;

    // Setter
    virtual void SetPosition(int posX, int poxY) = 0;
    virtual void SetScale(float scaleX, float scaleY) = 0;
    virtual void SetScale(float scale) = 0;
    virtual void SetSize(UINT width, UINT height) = 0;
    virtual void SetZ(float z) = 0;
};

interface IGameEngine
{
    virtual BOOL Initialize(HWND hWnd) = 0;

    virtual void PreUpdate(ULONGLONG curTick) = 0;
    virtual void Update(ULONGLONG curTick) = 0;
    virtual void LateUpdate(ULONGLONG curTick) = 0;

    virtual void Render() = 0;

    virtual void OnKeyDown(UINT nChar, UINT uiScanCode) = 0;
    virtual void OnKeyUp(UINT nChar, UINT uiScanCode) = 0;
    virtual void OnMouseMove(int mouseX, int mouseY) = 0;
    virtual BOOL OnUpdateWindowSize(UINT width, UINT height) = 0;

    virtual IGameObject *CreateGameObject() = 0;
    virtual void         DeleteGameObject(IGameObject * pGameObj) = 0;
    virtual void         DeleteAllGameObject() = 0;

    virtual IModel *GetPrimitiveModel(PRIMITIVE_MODEL_TYPE type) = 0;
    virtual IModel *CreateModelFromFile(const WCHAR *basePath, const WCHAR *filename) = 0;
    virtual void    DeleteModel(IModel * pModel) = 0;
    virtual void    DeleteAllModel() = 0;

    virtual ISprite *CreateSpriteFromFile(const WCHAR *basePath, const WCHAR *filename, UINT width, UINT height) = 0;
    virtual ISprite *CreateDynamicSprite(UINT width, UINT height) = 0;
    virtual void     DeleteSprite(ISprite * pSprite) = 0;
    virtual void     DeleteAllSprite() = 0;

    virtual IRenderer *GetRenderer() const = 0;
};

extern "C" ENGINEMODULE_API BOOL CreateGameEngine(HWND hWnd, IGameEngine **ppOutGameEngine);
extern "C" ENGINEMODULE_API void DeleteGameEngine(IGameEngine *pGameEngine);
