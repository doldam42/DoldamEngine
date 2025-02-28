#pragma once

#include "BaseObject.h"

interface IRenderer;
interface IGameManager;
interface IRenderMesh;

struct FaceGroup
{
    UINT *pIndices = nullptr;
    UINT  numTriangles = 0;
    int   materialIndex = -1;
};

class MeshObject : public IGameMesh, public BaseObject
{
  private:
    MESH_TYPE m_meshType = MESH_TYPE_UNKNOWN;

    UINT m_vertexCount = 0;
    UINT m_faceGroupCount = 0;
    UINT m_maxFaceGroupCount = 0;

    BasicVertex   *m_pBasicVertices = nullptr;
    SkinnedVertex *m_pSkinnedVertices = nullptr;
    FaceGroup     *m_pFaceGroups = nullptr;

    IRenderMesh *m_pMeshHandle = nullptr;

  private:
    void Cleanup();

  public:
    BOOL Initialize(MESH_TYPE meshType) override;
    BOOL Initialize(const WCHAR *name, const Transform *pLocalTransform, int parentIndex, int childCount,
                    MESH_TYPE meshType);
    BOOL InitRenderComponent(IRenderer *pRnd, const Material *pMaterials);

    void BeginCreateMesh(const void *pVertices, UINT numVertices, UINT numFaceGroup) override;
    void InsertFaceGroup(const UINT *pIndices, UINT numTriangles, int materialIndex) override;
    void EndCreateMesh() override;

    BOOL UpdateMaterial(IRenderMaterial *pMaterial, UINT faceGroupIndex) override;

    virtual void ReadFile(FILE *fp) override;
    virtual void WriteFile(FILE *fp) override;

    void Render(IRenderer *pRnd, const Matrix *pWorldMat, const Matrix *pBoneMatrices, const UINT numJoints);
    void RenderWithMaterials(IRenderer *pRnd, const Matrix *pWorldMat, const Matrix *pBoneMatrices,
                             const UINT numJoints, IRenderMaterial **ppMaterials, const UINT numMaterial);

    inline BOOL IsSkinned() const { return m_meshType == MESH_TYPE_SKINNED; }

    // Getter
    // if Skinned return nullptr
    inline BasicVertex   *GetBasicVertices() const { return m_pBasicVertices; }
    // if Basic return nullptr
    inline SkinnedVertex *GetSkinnedVertices() const { return m_pSkinnedVertices; }
    inline UINT           GetVertexCount() const { return m_vertexCount; }

    // IBaseObject에서 상속받음
    inline void SetName(const WCHAR *name) override { BaseObject::SetName(name); }
    inline void SetTransform(const Transform *pLocalTransform) override { BaseObject::SetTransform(pLocalTransform); };
    inline void SetParentIndex(int parentIndex) override { BaseObject::SetParentIndex(parentIndex); }
    inline void AddChildCount() override { BaseObject::AddChildCount(); }

    MeshObject() = default;
    MeshObject(const BaseObject &obj) : BaseObject(obj){};
    virtual ~MeshObject() override;
};
