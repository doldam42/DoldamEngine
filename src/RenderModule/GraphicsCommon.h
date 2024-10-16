#pragma once

// TODO: BIT FLAG 적용하기
#include "ConstantBuffers.h"

namespace Graphics
{

extern const wchar_t *hitGroupNames[2];
extern const wchar_t *hitShaderNames[2];
extern const wchar_t *missShaderNames[2];

enum GLOBAL_ROOT_PARAM_INDEX
{
    GLOBAL_ROOT_PARAM_INDEX_OUTPUTVIEW = 0,
    GLOBAL_ROOT_PARAM_INDEX_ACCELERATIONSTRUCTURE,
    GLOBAL_ROOT_PARAM_INDEX_CAMERA,
    GLOBAL_ROOT_PARAM_INDEX_COUNT
};

enum LOCAL_ROOT_PARAM_INDEX
{
    LOCAL_ROOT_PARAM_INDEX_CB = 0,
    LOCAL_ROOT_PARAM_INDEX_VERTICES,
    LOCAL_ROOT_PARAM_INDEX_INDICES,
    LOCAL_ROOT_PARAM_INDEX_DIFFUSE_TEX,
    LOCAL_ROOT_PARAM_INDEX_COUNT
};

enum SKINNING_ROOT_PARAM_INDEX
{
    SKINNING_ROOT_PARAM_INDEX_VERTEX = 0,
    SKINNING_ROOT_PARAM_INDEX_BONES,
    SKINNING_ROOT_PARAM_INDEX_COUNT
};

struct LOCAL_ROOT_ARG
{
    GeometryConstants           cb;
    D3D12_GPU_DESCRIPTOR_HANDLE vertices;
    D3D12_GPU_DESCRIPTOR_HANDLE indices;
    D3D12_GPU_DESCRIPTOR_HANDLE diffuseTex;
};

// RootSignature
extern ID3D12RootSignature *emptyRS;
extern ID3D12RootSignature *deformingVertexRS;
extern ID3D12RootSignature *presentRS;
extern ID3D12RootSignature *depthOnlyBasicRS;
extern ID3D12RootSignature *depthOnlySkinnedRS;

// extern ID3D12RootSignature *rootSignatures[RENDER_ITEM_TYPE_COUNT];

// Pipeline State Objects
extern ID3D12PipelineState *deformingVertexPSO;
extern ID3D12PipelineState *presentPSO;
extern ID3D12PipelineState *D32ToRgbaPSO;
// extern ID3D12PipelineState *PSO[RENDER_ITEM_TYPE_COUNT][DRAW_PASS_TYPE_COUNT][FILL_MODE_COUNT];

// #DXR
extern IDxcBlob *rayGenLibrary;
extern IDxcBlob *missLibrary;
extern IDxcBlob *hitLibrary;
extern IDxcBlob *shadowLibrary;

extern ID3D12RootSignature *globalRS;
extern ID3D12RootSignature *localHitRS;
extern ID3D12RootSignature *localEmptyRS;

extern ID3D12StateObject           *rtStateObject;
extern ID3D12StateObjectProperties *rtStateObjectProps;

void InitCommonStates(ID3D12Device5 *pD3DDevice);
void InitInputLayouts();
void InitBlendStates();
void InitRasterizerStates();
void InitDepthStencilStates();
void InitSamplers();
void InitShaders(ID3D12Device5 *pD3DDevice);
void InitRootSignature(ID3D12Device5 *pD3DDevice);
void InitPipelineStates(ID3D12Device5 *pD3DDevice);

// #DXR
HRESULT InitRaytracingStates(ID3D12Device5 *pD3DDevice);
void    InitRaytracingShaders(CD3DX12_STATE_OBJECT_DESC *raytracingPipeline);
void    InitRaytracingRootSignatures(ID3D12Device5 *pD3DDevice);
void    InitRaytracingStateObjects(CD3DX12_STATE_OBJECT_DESC *raytracingPipeline);

ID3D12RootSignature *GetRS(RENDER_ITEM_TYPE itemType);
ID3D12PipelineState *GetPSO(RENDER_ITEM_TYPE itemType, DRAW_PASS_TYPE passType, FILL_MODE fillMode);

void DeleteCommonStates();
void DeleteShaders();
void DeleteSamplers();
void DeleteRootSignatures();
void DeletePipelineStates();

void DeleteRaytracingStates();

void SerializeAndCreateRootSignature(ID3D12Device5 *pDevice, const D3D12_ROOT_SIGNATURE_DESC *pDesc,
                                     ID3D12RootSignature **ppOutRS, const WCHAR *rootSigName);

} // namespace Graphics