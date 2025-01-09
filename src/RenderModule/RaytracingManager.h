#pragma once

#include <d3d12.h>
#include "RendererTypedef.h"
#include "D3D12Renderer.h"
#include "GraphicsCommon.h"

struct DESCRIPTOR_HANDLE;
class ShaderTable;
class RaytracingManager
{
    CRITICAL_SECTION   m_cs = {};
    CONDITION_VARIABLE m_cv = {};

    D3D12Renderer *m_pRenderer = nullptr;

    AccelerationStructureBuffers m_topLevelASBuffers;
    DESCRIPTOR_HANDLE            m_TLASHandle = {};

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS m_topLevelInputs = {};
    D3D12_RAYTRACING_INSTANCE_DESC                      *m_pInstanceDescsCPU = nullptr;

    ShaderTable *m_pRayGenShaderTable = nullptr;
    ShaderTable *m_pMissShaderTable = nullptr;
    ShaderTable *m_pHitShaderTables[MAX_PENDING_FRAME_COUNT] = {nullptr};

    UINT m_AllocatedInstanceCount = 0;
    UINT m_maxInstanceCount = 0;

    UINT m_curContextIndex = 0;

    void CreateShaderTables();
    void Cleanup();

  public:
    BOOL Initialize(D3D12Renderer *pRnd, ID3D12GraphicsCommandList4 *pCommandList, UINT maxInstanceCount);

    void CreateTopLevelAS(ID3D12GraphicsCommandList4 *pCommandList);
    
    // Hit Group의 index를 반환함
    void InsertBLASInstance(ID3D12Resource *pBLAS, const Matrix *pTM, UINT instanceID, Graphics::LOCAL_ROOT_ARG* pRootArgs, UINT numFaceGroup);

    void DispatchRay(ID3D12GraphicsCommandList4 *pCommandList, ID3D12Resource *pOutputView,
                     D3D12_CPU_DESCRIPTOR_HANDLE outputViewSrv);

    void Reset();

    RaytracingManager() = default;
    ~RaytracingManager();
};