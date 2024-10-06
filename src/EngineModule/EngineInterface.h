#pragma once

#ifdef ENGINEMODULE_EXPORTS
#define ENGINEMODULE_API __declspec(dllexport)
#else
#define ENGINEMODULE_API __declspec(dllimport)
#endif

#include <combaseapi.h>
#include "../MathModule/MathHeaders.h"

interface IRenderer;
class IRenderer;
struct Material;
struct Joint;
struct Transform;
enum PRIMITIVE_MODEL_TYPE : UINT
{
    PRIMITIVE_MODEL_TYPE_SQUARE = 0,
    PRIMITIVE_MODEL_TYPE_BOX,
};

enum MESH_TYPE : UINT
{
    MESH_TYPE_UNKNOWN = 0,
    MESH_TYPE_DEFAULT,
    MESH_TYPE_SKINNED,
};

interface ISerializable
{
    virtual void WriteFile(FILE * fp) = 0;
    virtual void ReadFile(FILE * fp) = 0;
};

interface IBaseObject
{
    virtual void SetName(const WCHAR *name) = 0;
    virtual void SetTransform(const Transform *pLocalTransform) = 0;
    virtual void SetParentIndex(int parentIndex) = 0;
    virtual void AddChildCount() = 0;
};

interface IGameMesh : public IBaseObject
{
    virtual BOOL Initialize(MESH_TYPE meshType) = 0;
    virtual void BeginCreateMesh(const void *pVertices, UINT numVertices, UINT numFaceGroup) = 0;
    virtual void InsertFaceGroup(const UINT *pIndices, UINT numTriangles, int materialIdx) = 0;
    virtual void EndCreateMesh() = 0;
};

interface IGameModel : public IUnknown, public ISerializable
{
    virtual void Initialize(const Material *pInMaterial, int materialCount, IGameMesh **ppInObjs, int objectCount, Joint* pInJoints = nullptr, int jointCount = 0) = 0;
};

interface IGameObject
{
    virtual Vector3 GetPosition() = 0;
    virtual Vector3 GetScale() = 0;
    virtual float   GetRotationX() = 0;
    virtual float   GetRotationY() = 0;
    virtual float   GetRotationZ() = 0;

    virtual void SetModel(IGameModel * pModel) = 0;
    virtual void SetPosition(float x, float y, float z) = 0;
    virtual void SetScale(float x, float y, float z) = 0;
    virtual void SetScale(float s) = 0;
    virtual void SetRotationX(float rotX) = 0;
    virtual void SetRotationY(float rotY) = 0;
    virtual void SetRotationZ(float rotZ) = 0;
};

interface IGameSprite
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

interface IGameAnimation : public IUnknown, public ISerializable
{
    virtual void SetName(const WCHAR *name) = 0;
    virtual void BeginCreateAnim(int jointCount) = 0;
    virtual void InsertKeyframes(const wchar_t *bindingJointName, const Matrix *pInKeys, uint32_t numKeys) = 0;
    virtual void EndCreateAnim() = 0;
};

interface IInputManager
{
    virtual float GetXAxis() const = 0;
    virtual float GetYAxis() const = 0;
    virtual float GetZAxis() const = 0;

    virtual float GetCursorNDCX() const = 0;
    virtual float GetCursorNDCY() const = 0;

    virtual BOOL IsKeyPressed(UINT nChar) const = 0;
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

    virtual IGameModel *GetPrimitiveModel(PRIMITIVE_MODEL_TYPE type) = 0;
    virtual IGameModel *CreateModelFromFile(const WCHAR *basePath, const WCHAR *filename) = 0;
    virtual IGameModel *CreateEmptyModel() = 0;
    virtual void        DeleteModel(IGameModel * pModel) = 0;
    virtual void        DeleteAllModel() = 0;

    virtual IGameSprite *CreateSpriteFromFile(const WCHAR *basePath, const WCHAR *filename, UINT width,
                                              UINT height) = 0;
    virtual IGameSprite *CreateDynamicSprite(UINT width, UINT height) = 0;
    virtual void         DeleteSprite(IGameSprite * pSprite) = 0;
    virtual void         DeleteAllSprite() = 0;

    virtual IGameAnimation *CreateAnimationFromFile(const WCHAR *basePath, const WCHAR *filename) = 0;
    virtual IGameAnimation *CreateEmptyAnimation() = 0;
    virtual void            DeleteAnimation(IGameAnimation * pAnim) = 0;
    virtual void            DeleteAllAnimation() = 0;

    virtual IRenderer     *GetRenderer() const = 0;
    virtual IInputManager *GetInputManager() const = 0;
};

extern "C" ENGINEMODULE_API BOOL CreateGameEngine(HWND hWnd, IGameEngine **ppOutGameEngine);
extern "C" ENGINEMODULE_API void DeleteGameEngine(IGameEngine *pGameEngine);

extern "C" ENGINEMODULE_API BOOL CreateGameMesh(IGameMesh** ppOutMesh);
extern "C" ENGINEMODULE_API void DeleteGameMesh(IGameMesh* pInMesh);