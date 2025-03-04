#pragma once

#include "BaseObject.h"
#include "MeshObject.h"

class GameObject;
class Model : public IGameModel
{
    static size_t g_id;
    size_t m_id = 0;

    static const UINT MAX_INSTANCE_COUNT = 256;
    IRenderer        *m_pRenderer = nullptr;

    Matrix  m_defaultTransform = Matrix::Identity;
    Matrix *m_pBoneMatrices = nullptr;

    WCHAR m_basePath[MAX_PATH] = {L'\0'};
    ULONG ref_count = 1;

    // File I/O
    UINT m_objectCount;
    UINT m_materialCount;
    UINT m_jointCount;

    Material    *m_pMaterials = nullptr;
    IRenderMaterial **m_ppMaterials = nullptr;

    MeshObject **m_ppMeshObjects = nullptr;
    Joint       *m_pJoints = nullptr;

    Bounds m_boundingBox;
    IRenderMesh *m_pBoundingBoxMesh;

  public:
    SORT_LINK m_LinkInGame;
    void     *m_pSearchHandleInGame = nullptr;

  private:
    void Cleanup();

  public:
    void Initialize(const Material *pInMaterial, int materialCount, IGameMesh **ppInObjs, int objectCount,
                    Joint *pInJoint = nullptr, int jointCount = 0) override;
    void InitRenderComponents(IRenderer *pRenderer);

    void InitBoundary();

    void ReadFile(const char *filename) override;
    void WriteFile(const char *filename) override;

    void UpdateAnimation(AnimationClip *pClip, int frameCount);

    void Render(GameObject *pGameObj, IRenderMaterial **ppMaterials = nullptr, UINT numMaterials = 0);

    // Getter
    inline UINT        GetObjectCount() const { return m_objectCount; }
    inline MeshObject *GetObjectByIdx(UINT index) const { return m_ppMeshObjects[index]; }
    inline UINT        GetJointCount() const { return m_jointCount; }
    inline Joint      *GetJointByIdx(UINT index) const { return m_pJoints + index; }

    // Setter
    void SetBasePath(const WCHAR *basePath);

    size_t GetID() override { return m_id; }

    // Inherited via IModel
    HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject) override;
    ULONG __stdcall AddRef(void) override;
    ULONG __stdcall Release(void) override;

    inline IGameMesh *GetMeshAt(UINT index) override { return m_ppMeshObjects[index]; }

    Bounds& GetBoundingBox() { return m_boundingBox; }
    //Sphere& GetBoundingSphere() { return m_boundingSphere; }

    const Material *GetMaterials() { return m_pMaterials; }
    UINT            GetMaterialCount() { return m_materialCount; }

    Model();
    ~Model();
};
