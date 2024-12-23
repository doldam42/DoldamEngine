#pragma once

#include "../GenericModule/LinkedList.h"
#include "BaseObject.h"
#include "EngineInterface.h"
#include "MeshObject.h"

class GameObject;
class Model : public IGameModel
{
    static const UINT MAX_INSTANCE_COUNT = 256;
    IRenderer        *m_pRenderer = nullptr;

    Matrix  m_defaultTransform = Matrix::Identity;
    Matrix *m_pBoneMatrices = nullptr;

    WCHAR m_basePath[MAX_PATH] = {L'\0'};
    ULONG ref_count = 0;

    // File I/O
    UINT m_objectCount;
    UINT m_materialCount;
    UINT m_jointCount;

    Material    *m_pMaterials = nullptr;
    MeshObject **m_ppMeshObjects = nullptr;
    Joint       *m_pJoints = nullptr;

    // Sphere m_boundingSphere;

  public:
    SORT_LINK m_LinkInGame;

  private:
    void Cleanup();

  public:
    void Initialize(const Material *pInMaterial, int materialCount, IGameMesh **ppInObjs, int objectCount,
                    Joint *pInJoint = nullptr, int jointCount = 0) override;
    void InitRenderComponents(IRenderer *pRenderer);

    void InitBoundary();

    void ReadFile(FILE *fp);
    void WriteFile(FILE *fp);

    void UpdateAnimation(AnimationClip *pClip, int frameCount);
    void Render(GameObject *pGameObj);
    void RenderWithMaterials(GameObject* pGameObj, IRenderMaterial** ppMaterials, UINT numMaterials);

    // Getter
    inline UINT        GetObjectCount() const { return m_objectCount; }
    inline MeshObject *GetObjectByIdx(UINT index) const { return m_ppMeshObjects[index]; }
    inline UINT        GetJointCount() const { return m_jointCount; }
    inline Joint      *GetJointByIdx(UINT index) const { return m_pJoints + index; }

    // Setter
    void SetBasePath(const WCHAR *basePath);

    // Inherited via IModel
    HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject) override;
    ULONG __stdcall AddRef(void) override;
    ULONG __stdcall Release(void) override;

    inline IGameMesh *GetMeshAt(UINT index) override { return m_ppMeshObjects[index]; }
    Model();
    ~Model();
};
