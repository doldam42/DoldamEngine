#pragma once

#include "BasicObject.h"
#include "RenderModule.h"

enum VERTEX_TYPE
{
    VERTEX_TYPE_BASIC = 0,
    VERTEX_TYPE_SKINNED
};

struct FACE_GROUP_HEADER
{
    int MaterialIndex;
    int IndexCount;
};

struct FACE_GROUP
{
    FACE_GROUP_HEADER Header;
    uint32_t          pIndices[0];

    static inline FACE_GROUP *Alloc(FACE_GROUP_HEADER header)
    {
        FACE_GROUP *pFace = nullptr;
        pFace = (FACE_GROUP *)malloc(sizeof(FACE_GROUP) + sizeof(uint32_t) * header.IndexCount);

        if (!pFace)
            return nullptr;

        pFace->Header = header;
        return pFace;
    }

    static inline void Dealloc(FACE_GROUP *pStream) { free(pStream); }

    inline size_t GetSize() const { return sizeof(FACE_GROUP) + sizeof(uint32_t) * Header.IndexCount; }
    FACE_GROUP() = delete;
};

struct MESH_HEADER
{
    VERTEX_TYPE Type;
    int         VertexCount;
    int         FaceGroupCount;
};

class IRenderer;
class MeshObject : public BasicObject
{
    MESH_HEADER  m_header = {};
    uint8_t     *m_pVertices = nullptr;
    FACE_GROUP **m_ppFaceGroup = nullptr;

    virtual void InitCollisionBox();
    virtual void InitCollisionSphere();

  protected:
    IRenderer   *m_pRenderer = nullptr;
    IRenderMesh *m_pMeshHandle = nullptr;

    IRenderMesh *m_pCollisionBoxMesh = nullptr;
    IRenderMesh *m_pCollisionSphereMesh = nullptr;

    Box    m_boundingBox;
    Sphere m_boundingSphere;

    IRenderMesh *CreateWireBoxMesh(const Vector3 &center, const Vector3 &extends);
    IRenderMesh *CreateSphereMesh(float radius, int numSlices, int numStacks);

  private:
    void Cleanup();

  public:
    virtual void InitMeshHandle(IRenderer *pRnd, const Material *pMaterials, const wchar_t *basePath) override;

    void BeginCreateMesh(const void *pVertices, VERTEX_TYPE vertexType, uint32_t numVertices, uint32_t numFaceGroup);
    void InsertFaceGroup(const uint32_t *pIndices, uint32_t numIndices, int materialIdx);

    void WriteFile(FILE *fp) override;
    void ReadFile(FILE *fp) override;

    MeshObject() = delete;
    MeshObject(const OBJECT_HEADER &header) : BasicObject::BasicObject(header) {}

    inline uint32_t GetVertexCount() const { return m_header.VertexCount; }
    inline uint32_t GetFaceGroupCount() const { return m_header.FaceGroupCount; }

    inline const Box    &GetCollisionBox() const { return m_boundingBox; }
    inline const Sphere &GetCollisionSphere() const { return m_boundingSphere; }

    BasicVertex   *GetBasicVertices();
    SkinnedVertex *GetSkinnedVertices();
    FACE_GROUP    *GetFaceGroup(uint32_t index);

    virtual bool IsInFrustum(const Matrix &worldMat, const BoundingFrustum &frustum) const override;

    virtual void Render(IRenderer *pRnd, const Matrix &worldMat) override;

    virtual ~MeshObject() override;
};
