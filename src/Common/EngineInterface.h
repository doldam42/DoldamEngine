#pragma once

#ifdef ENGINEMODULE_EXPORTS
#define ENGINEMODULE_API __declspec(dllexport)
#else
#define ENGINEMODULE_API __declspec(dllimport)
#endif

#include <combaseapi.h>

#include "../Common/RendererInterface.h"
#include "../GenericModule/GenericHeaders.h"
#include "../MathModule/MathHeaders.h"

struct Material;
struct Joint;
struct Transform;

enum GAME_ITEM_TYPE : UINT
{
    GAME_ITEM_TYPE_NONE = 0,
    GAME_ITEM_TYPE_MODEL,
    GAME_ITEM_TYPE_GAME_OBJECT,
    GAME_ITEM_TYPE_GAME_CHARACTER,
};

enum PRIMITIVE_MODEL_TYPE : UINT
{
    PRIMITIVE_MODEL_TYPE_SQUARE = 0,
    PRIMITIVE_MODEL_TYPE_BOX,
    PRIMITIVE_MODEL_TYPE_SPHERE,
};

enum MESH_TYPE : UINT
{
    MESH_TYPE_UNKNOWN = 0,
    MESH_TYPE_DEFAULT,
    MESH_TYPE_SKINNED,
};

interface ISerializable
{
    virtual void WriteFile(const char *filename) = 0;
    virtual void ReadFile(const char *filename) = 0;
};

interface IController
{
    virtual BOOL Start() = 0;
    virtual void Update(float dt) = 0;
};

interface IRenderableController : public IController { virtual void Render() = 0; };

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
    virtual void       Initialize(const Material *pInMaterial, int materialCount, IGameMesh **ppInObjs, int objectCount,
                                  Joint *pInJoints = nullptr, int jointCount = 0) = 0;
    virtual size_t     GetID() = 0;
    virtual IGameMesh *GetMeshAt(UINT index) = 0;

    virtual void SetMaterials(IRenderMaterial * *ppMaterials, const UINT numMaterials) = 0;
};

enum COLLIDER_TYPE
{
    COLLIDER_TYPE_SPHERE = 0,
    COLLIDER_TYPE_BOX,
    COLLIDER_TYPE_CAPSULE,
    COLLIDER_TYPE_ELLIPSE,
    COLLIDER_TYPE_CONVEX,
};
interface ICollider
{
    virtual COLLIDER_TYPE GetType() const = 0;

    virtual void Update() = 0;

    virtual Vector3 GetCenter() const = 0;
    virtual Vector3 GetWorldCenter() const = 0;
    virtual Bounds  GetBounds() const = 0;
    virtual Bounds  GetWorldBounds() const = 0;

    virtual Matrix InertiaTensor() const = 0;

    virtual BOOL Intersect(ICollider * pOther) const = 0;
    virtual BOOL IntersectRay(const Ray &ray, float *hitt0, float *hitt1) const = 0;
    virtual BOOL Intersect(const Bounds &b) const = 0;

    // Find the point in furthest in direction
    virtual Vector3 Support(const Vector3 dir, const Vector3 pos, const Quaternion orient, const float bias) = 0;
    virtual float   FastestLinearSpeed(const Vector3 angularVelocity, const Vector3 dir) const = 0;
};

interface IRigidBody
{
    virtual Vector3 GetVelocity() const = 0;
    virtual void    ApplyImpulseLinear(const Vector3 &impulse) = 0;
    virtual void    ApplyImpulseAngular(const Vector3 &impulse) = 0;
    virtual BOOL    IsStatic() = 0;
};

interface IGameObject : public IBoundedObject
{
    virtual size_t GetID() = 0;

    virtual BOOL InitBoxCollider(const Vector3 &center, const Vector3 &extent) = 0;
    virtual BOOL InitSphereCollider(const Vector3 &center, const float radius) = 0;
    virtual BOOL InitConvexCollider() = 0;

    virtual BOOL InitRigidBody(float mass, float elasticity, float friction, BOOL useGravity = TRUE,
                               BOOL isKinematic = FALSE) = 0;

    virtual Vector3 GetPosition() = 0;
    virtual Vector3 GetScale() = 0;
    virtual Vector3 GetForward() = 0;
    virtual float   GetRotationX() = 0;
    virtual float   GetRotationY() = 0;
    virtual float   GetRotationZ() = 0;

    virtual Quaternion GetRotation() = 0;

    virtual void SetModel(IGameModel * pModel) = 0;
    virtual void SetPosition(float x, float y, float z) = 0;
    virtual void SetScale(float x, float y, float z) = 0;
    virtual void SetScale(float s) = 0;
    virtual void SetRotationX(float rotX) = 0;
    virtual void SetRotationY(float rotY) = 0;
    virtual void SetRotationZ(float rotZ) = 0;
    virtual void SetRotation(Quaternion q) = 0;

    virtual void AddPosition(Vector3 deltaPos) = 0;
    // virtual void Translate(const Vector3 *pInDeltaPos) = 0;

    virtual void             SetMaterials(IRenderMaterial * *ppMaterials, const UINT numMaterials) = 0;
    virtual IRenderMaterial *GetMaterialAt(UINT index) = 0;

    virtual ICollider  *GetCollider() const = 0;
    virtual IRigidBody *GetRigidBody() const = 0;
};

struct RayHit
{
    float        tHit;
    IGameObject *pHitted;
};

interface IGameAnimation : public IUnknown, public ISerializable
{
    virtual size_t GetID() = 0;
    virtual void   SetName(const WCHAR *name) = 0;
    virtual void   BeginCreateAnim(int jointCount) = 0;
    virtual void   InsertKeyframes(const wchar_t *bindingJointName, const Matrix *pInKeys, uint32_t numKeys) = 0;
    virtual void   EndCreateAnim() = 0;
};

interface IGameCharacter : public IGameObject { virtual void InsertAnimation(IGameAnimation * pClip) = 0; };

interface IGameSprite
{
    virtual BOOL UpdateTextureWithImage(const BYTE *pSrcBits, UINT srcWidth, UINT srcHeight) = 0;

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

interface IGameManager : public IUnknown
{
    virtual BOOL Initialize(HWND hWnd, IRenderer * pRnd, bool useGUIEditor = false, UINT viewportWidth = 0,
                            UINT viewportHeight = 0) = 0;

    virtual void Start() = 0;
    virtual void Update(float dt) = 0;
    virtual void Render() = 0;

    virtual void BuildScene() = 0;

    virtual BOOL OnUpdateWindowSize(UINT width, UINT height, UINT viewportWidth = 0, UINT viewportHeight = 0) = 0;

    virtual IGameMesh *CreateGameMesh() = 0;
    virtual void       DeleteGameMesh(IGameMesh * pGameMesh) = 0;

    virtual IGameCharacter *CreateCharacter() = 0;
    virtual IGameObject    *CreateGameObject() = 0;
    virtual void            DeleteGameObject(IGameObject * pGameObj) = 0;
    virtual void            DeleteAllGameObject() = 0;

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
    virtual IGameAnimation *CreateEmptyAnimation(const WCHAR *name) = 0;
    virtual IGameAnimation *GetAnimationByName(const WCHAR *name) = 0;
    virtual void            DeleteAnimation(IGameAnimation * pAnim) = 0;
    virtual void            DeleteAllAnimation() = 0;

    virtual BOOL CreateTerrain(const Material *pMaterial, const Vector3 *pScale, const int numSlice = 1,
                               const int numStack = 1) = 0;

    virtual void Register(IController * pController) = 0;
    virtual void Register(IRenderableController * pController) = 0;

    virtual Vector3 GetCameraPos() = 0;
    virtual Vector3 GetCameraLookTo() = 0;

    virtual void SetCameraPosition(float x, float y, float z) = 0;
    virtual void SetCameraYawPitchRoll(float yaw, float pitch, float roll) = 0;
    virtual void SetCameraEyeAtUp(Vector3 eye, Vector3 at, Vector3 up) = 0;

    virtual float DeltaTime() const = 0;

    virtual BOOL Raycast(const Vector3 rayOrigin, const Vector3 rayDir, RayHit *pOutHit,
                         float maxDistance = FLT_MAX) = 0;

    virtual IRenderer *GetRenderer() const = 0;
};
