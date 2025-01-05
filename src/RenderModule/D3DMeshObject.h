#pragma once

#include "GraphicsCommon.h"

struct MATERIAL_HANDLE;

enum BASIC_MESH_DESCRIPTOR_INDEX_PER_OBJ
{
    BASIC_DESCRIPTOR_INDEX_PER_OBJ_TM = 0,
    BASIC_DESCRIPTOR_INDEX_PER_OBJ_COUNT
};

enum SKINNED_MESH_DESCRIPTOR_INDEX_PER_OBJ
{
    SKINNED_DESCRIPTOR_INDEX_PER_OBJ_TM = 0,
    SKINNED_DESCRIPTOR_INDEX_PER_OBJ_BONES,
    SKINNED_DESCRIPTOR_INDEX_PER_OBJ_COUNT
};

enum DESCRIPTOR_INDEX_PER_FACE_GROUP
{
    DESCRIPTOR_INDEX_PER_FACE_GROUP_MATERIAL = 0,
    DESCRIPTOR_INDEX_PER_FACE_GROUP_TEX_ALBEDO,
    DESCRIPTOR_INDEX_PER_FACE_GROUP_TEX_NORMAL,
    DESCRIPTOR_INDEX_PER_FACE_GROUP_TEX_AO,
    DESCRIPTOR_INDEX_PER_FACE_GROUP_TEX_METALLIC_ROUGHNESS,
    DESCRIPTOR_INDEX_PER_FACE_GROUP_TEX_EMISSIVE,
    DESCRIPTOR_INDEX_PER_FACE_GROUP_COUNT
};

class D3D12Renderer;
class ShaderRecord;
struct DESCRIPTOR_HANDLE;

class D3DMeshObject : public IRenderMesh
{
    struct INDEXED_FACE_GROUP
    {
        DRAW_PASS_TYPE          passType = DRAW_PASS_TYPE_DEFAULT;
        ID3D12Resource         *pIndexBuffer = nullptr;
        D3D12_INDEX_BUFFER_VIEW IndexBufferView = {};
        UINT                    numTriangles = 0;
        MATERIAL_HANDLE        *pMaterialHandle = nullptr;
    };

  public:
    static const UINT DESCRIPTOR_COUNT_PER_STATIC_OBJ = 1;  // | World TM |
    static const UINT DESCRIPTOR_COUNT_PER_DYNAMIC_OBJ = 2; // | World TM | Bone Matrices |
    static const UINT MAX_FACE_GROUP_COUNT_PER_OBJ = 12;
    static const UINT MAX_DESCRIPTOR_COUNT_PER_DRAW_STATIC =
        DESCRIPTOR_COUNT_PER_STATIC_OBJ + (DESCRIPTOR_INDEX_PER_FACE_GROUP_COUNT * MAX_FACE_GROUP_COUNT_PER_OBJ);
    static const UINT MAX_DESCRIPTOR_COUNT_PER_DRAW_DYNAMIC =
        DESCRIPTOR_COUNT_PER_DYNAMIC_OBJ + (DESCRIPTOR_INDEX_PER_FACE_GROUP_COUNT * MAX_FACE_GROUP_COUNT_PER_OBJ);

  private:
    D3D12Renderer *m_pRenderer = nullptr;
    ID3D12Device  *m_pD3DDevice = nullptr;

    ID3D12Resource          *m_pVertexBuffer = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};

    INDEXED_FACE_GROUP *m_pFaceGroups = nullptr;
    UINT                m_faceGroupCount = 0;
    UINT                m_maxFaceGroupCount = 0;

    UINT m_indexCount = 0; // Number of indiecs = 3 * number of triangles
    UINT m_vertexCount = 0;

    UINT m_descriptorSize = 0;
    UINT m_descriptorCountPerDraw = 0;

    ULONG m_refCount = 0;

    RENDER_ITEM_TYPE m_type;

    Bounds m_bounds;

  private:
    void RenderNormal(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList, const Matrix *pWorldMat,
                      const Matrix *pBoneMats, UINT numBones, FILL_MODE fillMode = FILL_MODE_SOLID,
                      UINT numInstance = 1);

    void UpdateDescriptorTablePerObj(D3D12_CPU_DESCRIPTOR_HANDLE descriptorTable, UINT threadIndex,
                                     const Matrix *pWorldMat, UINT numInstance, const Matrix *pBoneMats, UINT numBones);
    void UpdateDescriptorTablePerFaceGroup(D3D12_CPU_DESCRIPTOR_HANDLE descriptorTable, UINT threadIndex, IRenderMaterial** ppMaterials, UINT numMaterial);

    void InitMaterial(INDEXED_FACE_GROUP *pFace, const Material *pInMaterial);
    void CleanupMaterial(INDEXED_FACE_GROUP *pFace);

    void CleanupMesh();
    void Cleanup();

  public:
    BOOL Initialize(D3D12Renderer *pRenderer, RENDER_ITEM_TYPE type);

    void Draw(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList, const Matrix *pWorldMat,
              IRenderMaterial **ppMaterials, UINT numMaterials, ID3D12RootSignature *pRS,
              ID3D12PipelineState *pPSO, D3D12_GPU_DESCRIPTOR_HANDLE globalCBV, const Matrix *pBoneMats, UINT numBones,
              DRAW_PASS_TYPE passType, UINT numInstance = 1);

    RENDER_ITEM_TYPE GetType() const { return m_type; }
    DRAW_PASS_TYPE   GetPassType() const { return m_pFaceGroups[0].passType; }

    const Bounds &GetBounds() { return m_bounds; }

    D3DMeshObject() = default;
    ~D3DMeshObject();

    // Inherited via IDIMeshObject
    HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject) override;
    ULONG __stdcall AddRef(void) override;
    ULONG __stdcall Release(void) override;

    BOOL BeginCreateMesh(const void *pVertices, UINT numVertices, UINT numFaceGroup) override;
    BOOL InsertFaceGroup(const UINT *pIndices, UINT numTriangles, const Material *pInMaterial,
                         const wchar_t *path) override;
    void EndCreateMesh() override;

    BOOL UpdateMaterial(IRenderMaterial *pInMaterial, UINT faceGroupIndex) override;
};
