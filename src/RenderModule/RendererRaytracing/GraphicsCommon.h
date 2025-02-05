#pragma once

// TODO: BIT FLAG 적용하기
#include <d3d12.h>
#include <d3dx12.h>
#include <dxcapi.h>
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
    GLOBAL_ROOT_PARAM_INDEX_DIFFUSE,
    GLOBAL_ROOT_PARAM_INDEX_NORMAL,
    GLOBAL_ROOT_PARAM_INDEX_ELEMENTS,
    GLOBAL_ROOT_PARAM_INDEX_DEPTH,
    GLOBAL_ROOT_PARAM_INDEX_COUNT
};

enum LOCAL_ROOT_PARAM_INDEX
{
    LOCAL_ROOT_PARAM_INDEX_CB = 0,
    LOCAL_ROOT_PARAM_INDEX_VERTICES,
    LOCAL_ROOT_PARAM_INDEX_INDICES,
    LOCAL_ROOT_PARAM_INDEX_TEXTURES,
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
    RaytracingFaceGroupCB       cb;
    D3D12_GPU_DESCRIPTOR_HANDLE vertices;
    D3D12_GPU_DESCRIPTOR_HANDLE indices;
    D3D12_GPU_DESCRIPTOR_HANDLE textures;
};

enum ADDITIONAL_PIPELINE_TYPE
{
    ADDITIONAL_PIPELINE_TYPE_POST_PROCESS,
    ADDITIONAL_PIPELINE_TYPE_COUNT
};

struct GraphicsShaderSet
{
    D3D12_INPUT_LAYOUT_DESC inputLayout;
    D3D12_SHADER_BYTECODE   VS;
    D3D12_SHADER_BYTECODE   PS;
    D3D12_SHADER_BYTECODE   HS;
    D3D12_SHADER_BYTECODE   DS;
    D3D12_SHADER_BYTECODE   GS;
};

static const D3D12_INPUT_ELEMENT_DESC terrainIEs[] = {
    {"HEIGHT", 0, DXGI_FORMAT_R8_UNORM, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};
static const D3D12_INPUT_ELEMENT_DESC simpleIEs[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

static const D3D12_INPUT_ELEMENT_DESC basicIEs[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};
static const D3D12_INPUT_ELEMENT_DESC skinnedIEs[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 60, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
};

static const D3D12_INPUT_LAYOUT_DESC simpleIL = {simpleIEs, _countof(simpleIEs)};
static const D3D12_INPUT_LAYOUT_DESC basicIL = {basicIEs, _countof(basicIEs)};
static const D3D12_INPUT_LAYOUT_DESC skinnedIL = {skinnedIEs, _countof(skinnedIEs)};
static const D3D12_INPUT_LAYOUT_DESC terrainIL = {terrainIEs, _countof(terrainIEs)};

extern GraphicsShaderSet g_shaderData[RENDER_ITEM_TYPE_COUNT][DRAW_PASS_TYPE_COUNT];
extern GraphicsShaderSet g_additionalShaderData[ADDITIONAL_PIPELINE_TYPE_COUNT];

// additional root signature
extern ID3D12RootSignature *basicRS;
extern ID3D12RootSignature *skinnedRS;
extern ID3D12RootSignature *spriteRS;

extern ID3D12RootSignature *emptyRS;
extern ID3D12RootSignature *presentRS;
extern ID3D12RootSignature *skyboxRS;
extern ID3D12RootSignature *deformingVertexRS;
extern ID3D12RootSignature *depthOnlyBasicRS;
extern ID3D12RootSignature *depthOnlySkinnedRS;

extern ID3D12RootSignature *secondPassRS;

// Pipeline State Objects
extern ID3D12PipelineState *deformingVertexPSO;
extern ID3D12PipelineState *presentPSO;
// extern ID3D12PipelineState *D32ToRgbaPSO;
extern ID3D12PipelineState *drawNormalPSO;
extern ID3D12PipelineState *drawSkinnedNormalPSO;
extern ID3D12PipelineState *skyboxPSO;

extern ID3D12PipelineState *secondPassPSO;

// #DXR
extern IDxcBlob *rayGenLibrary;
extern IDxcBlob *missLibrary;
extern IDxcBlob *hitLibrary;
extern IDxcBlob *shadowLibrary;
extern IDxcBlob *raytracingLibrary;

extern ID3D12RootSignature *globalRS;
extern ID3D12RootSignature *localHitRS;
extern ID3D12RootSignature *localEmptyRS;

extern ID3D12StateObject           *rtStateObject;
extern ID3D12StateObjectProperties *rtStateObjectProps;

void InitCommonStates(ID3D12Device5 *pD3DDevice);
void InitSamplers();
void InitShaders(ID3D12Device5 *pD3DDevice);
void InitRootSignature(ID3D12Device5 *pD3DDevice);
void InitPipelineStates(ID3D12Device5 *pD3DDevice);

// #DXR
HRESULT InitRaytracingStates(ID3D12Device5 *pD3DDevice);
void    InitRaytracingShaders(CD3DX12_STATE_OBJECT_DESC *raytracingPipeline);
void    InitRaytracingRootSignatures(ID3D12Device5 *pD3DDevice);
void    InitRaytracingStateObjects(CD3DX12_STATE_OBJECT_DESC *raytracingPipeline);

ID3D12RootSignature *GetRS(RENDER_ITEM_TYPE itemType, DRAW_PASS_TYPE passType);
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