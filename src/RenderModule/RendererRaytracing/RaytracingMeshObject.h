#pragma once

#include <d3d12.h>

#include "../MathModule/MathHeaders.h"
#include "../GenericModule/GenericHeaders.h"

#include "../Common/RendererInterface.h"
#include "GraphicsCommon.h"

struct MATERIAL_HANDLE;

static constexpr UINT DESCRIPTOR_COUNT_PER_MATERIAL = 7;
static constexpr UINT DESCRIPTOR_COUNT_PER_BASIC_OBJECT = 1;
static constexpr UINT DESCRIPTOR_COUNT_PER_SKINNED_OBJECT = 2;
static constexpr UINT DESCRIPTOR_COUNT_PER_OIT_FRAGMENT_LIST = 4;

enum ROOT_ARG_DESCRIPTOR_INDEX_PER_BLAS
{
    ROOT_ARG_DESCRIPTOR_INDEX_PER_BLAS_VERTEX = 0,
    ROOT_ARG_DESCRIPTOR_INDEX_PER_BLAS_COUNT
};

enum ROOT_ARG_DESCRIPTOR_INDEX_PER_FACEGROUP
{
    ROOT_ARG_DESCRIPTOR_INDEX_PER_FACEGROUP_INDEX = 0,
    ROOT_ARG_DESCRIPTOR_INDEX_PER_FACEGROUP_TEX_ALBEDO,
    ROOT_ARG_DESCRIPTOR_INDEX_PER_FACEGROUP_TEX_NORMAL,
    ROOT_ARG_DESCRIPTOR_INDEX_PER_FACEGROUP_TEX_AO,
    ROOT_ARG_DESCRIPTOR_INDEX_PER_FACEGROUP_TEX_METALLIC_ROUGHNESS,
    ROOT_ARG_DESCRIPTOR_INDEX_PER_FACEGROUP_TEX_EMISSIVE,
    ROOT_ARG_DESCRIPTOR_INDEX_PER_FACEGROUP_TEX_HEIGHT,
    ROOT_ARG_DESCRIPTOR_INDEX_PER_FACEGROUP_COUNT
};

enum DESCRIPTOR_INDEX_PER_FACE_GROUP
{
    DESCRIPTOR_INDEX_PER_FACE_GROUP_MATERIAL = 0,
    DESCRIPTOR_INDEX_PER_FACE_GROUP_TEX_ALBEDO,
    DESCRIPTOR_INDEX_PER_FACE_GROUP_TEX_NORMAL,
    DESCRIPTOR_INDEX_PER_FACE_GROUP_TEX_AO,
    DESCRIPTOR_INDEX_PER_FACE_GROUP_TEX_METALLIC_ROUGHNESS,
    DESCRIPTOR_INDEX_PER_FACE_GROUP_TEX_EMISSIVE,
    DESCRIPTOR_INDEX_PER_FACE_GROUP_TEX_HEIGHT,
    DESCRIPTOR_INDEX_PER_FACE_GROUP_COUNT
};

enum SKINNING_DESCRIPTOR_INDEX
{
    SKINNING_DESCRIPTOR_INDEX_SRV = 0,
    SKINNING_DESCRIPTOR_INDEX_UAV,
    SKINNING_DESCRIPTOR_INDEX_COUNT
};

class D3D12Renderer;
class ShaderRecord;
struct DESCRIPTOR_HANDLE;

class RaytracingMeshObject : public IRenderMesh
{
    struct INDEXED_FACE_GROUP
    {
        ID3D12Resource         *pIndexBuffer = nullptr;
        D3D12_INDEX_BUFFER_VIEW IndexBufferView = {};
        UINT                    numTriangles = 0;
    };
  public:
    static const UINT DESCRIPTOR_COUNT_PER_STATIC_OBJ = 1;  // | World TM |
    static const UINT DESCRIPTOR_COUNT_PER_DYNAMIC_OBJ = 2; // | World TM | Bone Matrices |
    static const UINT MAX_FACE_GROUP_COUNT_PER_OBJ = 12;
    
    static const UINT DESCRIPTOR_COUNT_PER_BLAS = 1;         // | VertexBuffer |
    static const UINT DESCRIPTOR_COUNT_PER_RAY_FACEGROUP =
        ROOT_ARG_DESCRIPTOR_INDEX_PER_FACEGROUP_COUNT; // | IndexBuffer | Material Textures |
    static const UINT MAX_DESCRIPTOR_COUNT_PER_BLAS =
        DESCRIPTOR_COUNT_PER_BLAS + DESCRIPTOR_COUNT_PER_RAY_FACEGROUP * MAX_FACE_GROUP_COUNT_PER_OBJ; // TODO

  private:
    D3D12Renderer *m_pRenderer = nullptr;
    ID3D12Device5 *m_pD3DDevice = nullptr;

    ID3D12Resource          *m_pVertexBuffer = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};

    INDEXED_FACE_GROUP *m_pFaceGroups = nullptr;
    UINT                m_faceGroupCount = 0;
    UINT                m_maxFaceGroupCount = 0;

    IRenderMaterial **m_ppMaterials = nullptr;

    Matrix *m_pBoneMatrices = nullptr;
    Joint *m_pJoints = nullptr;
    UINT m_jointCount = 0;

    UINT m_vertexCount = 0;

    UINT m_descriptorSize = 0;
    UINT m_descriptorCountPerDraw = 0;

    ULONG m_refCount = 0;
    
    RENDER_ITEM_TYPE m_type = RENDER_ITEM_TYPE_MESH_OBJ;

    ID3D12Resource   *m_pDeformedVertexBuffer = nullptr;
    DESCRIPTOR_HANDLE m_skinningDescriptors = {}; // Vertex Buffer (SRV) | Vertex Buffer (UAV)

    AccelerationStructureBuffers    m_bottomLevelAS;
    D3D12_RAYTRACING_GEOMETRY_DESC *m_pBLASGeometries = nullptr;
    Graphics::LOCAL_ROOT_ARG       *m_pRootArgPerGeometries = nullptr;
    DESCRIPTOR_HANDLE               m_rootArgDescriptorTable = {};
    UINT64                          m_BLASScratchSizeInBytes = 0;
    UINT64                          m_BLASResultSizeInBytes = 0;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS m_BLASFlags = {};

  private:
    void UpdateBoneMatrices(Keyframe** ppKeyframes, UINT frameCount);

    void UpdateDescriptorTablePerObj(D3D12_CPU_DESCRIPTOR_HANDLE descriptorTable, UINT threadIndex,
                                     const Matrix *pWorldMat, UINT numInstance, Keyframe **ppKeyframes,
                                     UINT frameCount);
    void UpdateDescriptorTablePerFaceGroup(D3D12_CPU_DESCRIPTOR_HANDLE descriptorTable, UINT threadIndex,
                                           IRenderMaterial *const *ppMaterials, UINT numMaterial);

    void AddBLASGeometry(UINT faceGroupIndex, ID3D12Resource *vertexBuffer, UINT64 vertexOffsetInBytes,
                         uint32_t vertexCount, UINT vertexSizeInBytes, ID3D12Resource *indexBuffer,
                         UINT64 indexOffsetInBytes, uint32_t indexCount, ID3D12Resource *transformBuffer,
                         UINT64 transformOffsetInBytes, bool isOpaque = true);

    BOOL BuildBottomLevelAS(ID3D12GraphicsCommandList4 *commandList, ID3D12Resource *scratchBuffer,
                            ID3D12Resource *resultBuffer, bool isUpdate = false,
                            ID3D12Resource *previousResult = nullptr);

    void CreateSkinningBufferSRVs();
    void CreateRootArgsSRV();

    // 현재는 Prebuild한 resource크기의 2배를 미리 잡아놓는다.
    BOOL CreateBottomLevelAS(ID3D12GraphicsCommandList4 *pCommandList);
    void DeformingVerticesUAV(ID3D12GraphicsCommandList4 *pCommandList, Keyframe **ppKeyframes, UINT frameCount);

    void UpdateSkinnedBLAS(ID3D12GraphicsCommandList4 *pCommandList, Keyframe **ppKeyframes, UINT frameCount);

    void CleanupMesh();
    void Cleanup();

  public:
    BOOL Initialize(D3D12Renderer *pRenderer, RENDER_ITEM_TYPE type);

    void Draw(UINT threadIndex, ID3D12GraphicsCommandList4 *pCommandList, const Matrix *pWorldMat,
              DRAW_PASS_TYPE passType, FILL_MODE fillMode, Keyframe **ppKeyframes = nullptr, UINT frameCount = 0);

    void DrawWithMaterial(UINT threadIndex, ID3D12GraphicsCommandList4 *pCommandList, const Matrix *pWorldMat,
                          IRenderMaterial *const *ppMaterials, UINT numMaterials, DRAW_PASS_TYPE passType,
                          FILL_MODE fillMode, Keyframe **ppKeyframes = nullptr, UINT frameCount = 0);

    void RecordBlasInstance(UINT threadIndex, ID3D12GraphicsCommandList4 *pCommandList, const Matrix *pWorldMat,
              IRenderMaterial *const *ppMaterials, UINT numMaterials, Keyframe **ppKeyframes, UINT frameCount);

    ID3D12Resource *GetBottomLevelAS() const { return m_bottomLevelAS.pResult; }
    UINT            GetFaceGroupCount() const { return m_faceGroupCount; }

    // Inherited via IDIMeshObject
    HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject) override;
    ULONG __stdcall AddRef(void) override;
    ULONG __stdcall Release(void) override;

    BOOL BeginCreateMesh(const void *pVertices, UINT numVertices, const Joint *pJoint, UINT numJoint,
                         UINT numFaceGroup) override;
    BOOL InsertFaceGroup(const UINT *pIndices, UINT numTriangles, IRenderMaterial* pMaterial) override;
    void EndCreateMesh() override;
    void EndCreateMesh(ID3D12GraphicsCommandList4 *pCommandList);

    RaytracingMeshObject() = default;
    ~RaytracingMeshObject();
};
