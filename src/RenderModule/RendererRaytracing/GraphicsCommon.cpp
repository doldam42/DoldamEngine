#include "pch.h"

#include <d3dcompiler.h>

#include "D3D12Renderer.h"

#include "GraphicsCommon.h"

#define SizeOfInUint32(obj) ((sizeof(obj) - 1) / sizeof(UINT32) + 1)

namespace Graphics
{
// Compute Shader
ID3DBlob *deformingVertexCS = nullptr;

// Sampler States
D3D12_STATIC_SAMPLER_DESC samplerStates[SAMPLER_TYPE_COUNT] = {};

GraphicsShaderSet g_shaderData[RENDER_ITEM_TYPE_COUNT][DRAW_PASS_TYPE_COUNT] = {};
GraphicsShaderSet g_additionalShaderData[ADDITIONAL_PIPELINE_TYPE_COUNT] = {};

// Shaders
ID3DBlob *basicVS = nullptr;
ID3DBlob *basicSkinnedVS = nullptr;
ID3DBlob *basicPS = nullptr;

ID3DBlob *depthOnlyVS = nullptr;
ID3DBlob *depthOnlySkinnedVS = nullptr;
ID3DBlob *depthOnlyPS = nullptr;

ID3DBlob *spriteVS = nullptr;
ID3DBlob *spritePS = nullptr;

ID3DBlob *deferredPS = nullptr;
ID3DBlob *spriteDeferredPS = nullptr;

ID3DBlob *transparencyPS = nullptr;
ID3DBlob *transparentSpritePS = nullptr;

ID3DBlob *OITReslovePS = nullptr;

ID3DBlob *skyboxVS = nullptr;
ID3DBlob *skyboxPS = nullptr;

ID3DBlob *drawNormalVS = nullptr;
ID3DBlob *drawNormalSkinnedVS = nullptr;
ID3DBlob *drawNormalGS = nullptr;
ID3DBlob *drawNormalPS = nullptr;

ID3DBlob *presentVS = nullptr;
ID3DBlob *presentPS = nullptr;

ID3DBlob *lightOnlyPS = nullptr;

// Tessellation
IDxcBlob *tessellatedQuadVS = nullptr;
IDxcBlob *tessellatedQuadHS = nullptr;
IDxcBlob *tessellatedQuadDS = nullptr;
IDxcBlob *tessellatedQuadPS = nullptr;
IDxcBlob *tessellatedQuadDeferredPS = nullptr;

// Terrain
IDxcBlob *terrainVS = nullptr;
IDxcBlob *terrainHS = nullptr;
IDxcBlob *terrainDS = nullptr;
IDxcBlob *terrainPS = nullptr;
IDxcBlob *terrainDeferredPS = nullptr;

// RootSignature
ID3D12RootSignature *basicRS = nullptr;
ID3D12RootSignature *skinnedRS = nullptr;
ID3D12RootSignature *spriteRS = nullptr;

ID3D12RootSignature *transparentBasicRS = nullptr;
ID3D12RootSignature *transparentSkinnedRS = nullptr;
ID3D12RootSignature *transparentSpriteRS = nullptr;

ID3D12RootSignature *OITResolveRS = nullptr;

ID3D12RootSignature *emptyRS = nullptr;
ID3D12RootSignature *skyboxRS = nullptr;
ID3D12RootSignature *presentRS = nullptr;
ID3D12RootSignature *depthOnlyBasicRS = nullptr;
ID3D12RootSignature *depthOnlySkinnedRS = nullptr;
ID3D12RootSignature *deformingVertexRS = nullptr;

// Deferred
ID3D12RootSignature *secondPassRS = nullptr;

ID3D12RootSignature *rootSignatures[RENDER_ITEM_TYPE_COUNT][DRAW_PASS_TYPE_COUNT] = {nullptr};

// Pipeline State Objects
ID3D12PipelineState *deformingVertexPSO = nullptr;
ID3D12PipelineState *presentPSO = nullptr;
ID3D12PipelineState *D32ToRgbaPSO = nullptr;
ID3D12PipelineState *drawNormalPSO = nullptr;
ID3D12PipelineState *drawSkinnedNormalPSO = nullptr;
ID3D12PipelineState *skyboxPSO = nullptr;

// Deferred
ID3D12PipelineState *secondPassPSO = nullptr;

ID3D12PipelineState *PSO[RENDER_ITEM_TYPE_COUNT][DRAW_PASS_TYPE_COUNT][FILL_MODE_COUNT] = {nullptr};

// #DXR
const wchar_t *missShaderNames[2] = {L"Miss", L"ShadowMiss"};
const wchar_t *hitShaderNames[2] = {L"ClosestHit", L"ShadowClosestHit"};
const wchar_t *hitGroupNames[2] = {L"HitGroup", L"ShadowHitGroup"};

IDxcBlob *rayGenLibrary = nullptr;
IDxcBlob *missLibrary = nullptr;
IDxcBlob *hitLibrary = nullptr;
IDxcBlob *shadowLibrary = nullptr;
IDxcBlob *raytracingLibrary = nullptr;

ID3D12RootSignature *globalRS = nullptr;
ID3D12RootSignature *localHitRS = nullptr;
ID3D12RootSignature *localEmptyRS = nullptr;

ID3D12StateObject           *rtStateObject = nullptr;
ID3D12StateObjectProperties *rtStateObjectProps = nullptr;
} // namespace Graphics

void Graphics::InitCommonStates(ID3D12Device5 *pD3DDevice)
{
    InitSamplers();

    InitShaders(pD3DDevice);
    InitRootSignature(pD3DDevice);
    InitPipelineStates(pD3DDevice);

    InitRaytracingStates(pD3DDevice);
}

void Graphics::InitShaders(ID3D12Device5 *pD3DDevice)
{
    HRESULT hr = S_OK;

#if defined(_DEBUG)
    // Enable better shader debugging with the graphics debugging tools.
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif
    D3D_SHADER_MACRO skinnedVSMacros[2] = {{"SKINNED", "1"}, {NULL, NULL}};

    // Mesh Object
    hr = D3DCompileFromFile(L"./Shaders/BasicVS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1",
                            compileFlags, 0, &basicVS, nullptr);
    if (FAILED(hr))
        __debugbreak();
    hr = D3DCompileFromFile(L"./Shaders/BasicPS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1",
                            compileFlags, 0, &basicPS, nullptr);
    if (FAILED(hr))
        __debugbreak();
    hr = D3DCompileFromFile(L"./Shaders/BasicVS.hlsl", skinnedVSMacros, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
                            "vs_5_1", compileFlags, 0, &basicSkinnedVS, nullptr);
    if (FAILED(hr))
        __debugbreak();

    // SkyBox
    hr = D3DCompileFromFile(L"./Shaders/SkyboxVS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1",
                            compileFlags, 0, &skyboxVS, nullptr);
    if (FAILED(hr))
        __debugbreak();
    hr = D3DCompileFromFile(L"./Shaders/SkyboxPS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1",
                            compileFlags, 0, &skyboxPS, nullptr);

    // Sprite
    hr = D3DCompileFromFile(L"./Shaders/SpriteVS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1",
                            compileFlags, 0, &spriteVS, nullptr);
    if (FAILED(hr))
        __debugbreak();
    hr = D3DCompileFromFile(L"./Shaders/SpritePS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1",
                            compileFlags, 0, &spritePS, nullptr);
    if (FAILED(hr))
        __debugbreak();

    // Depth Only
    hr = D3DCompileFromFile(L"./Shaders/DepthOnlyVS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1",
                            compileFlags, 0, &depthOnlyVS, nullptr);
    if (FAILED(hr))
        __debugbreak();
    hr = D3DCompileFromFile(L"./Shaders/DepthOnlyVS.hlsl", skinnedVSMacros, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
                            "vs_5_1", compileFlags, 0, &depthOnlySkinnedVS, nullptr);
    if (FAILED(hr))
        __debugbreak();
    hr = D3DCompileFromFile(L"./Shaders/DepthOnlyPS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1",
                            compileFlags, 0, &depthOnlyPS, nullptr);
    if (FAILED(hr))
        __debugbreak();

    // Draw Normal
    hr = D3DCompileFromFile(L"./Shaders/DrawNormal.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain",
                            "vs_5_1", compileFlags, 0, &drawNormalVS, nullptr);
    if (FAILED(hr))
        __debugbreak();
    hr = D3DCompileFromFile(L"./Shaders/DrawNormal.hlsl", skinnedVSMacros, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain",
                            "vs_5_1", compileFlags, 0, &drawNormalSkinnedVS, nullptr);
    if (FAILED(hr))
        __debugbreak();

    hr = D3DCompileFromFile(L"./Shaders/DrawNormal.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "GSMain",
                            "gs_5_1", compileFlags, 0, &drawNormalGS, nullptr);
    if (FAILED(hr))
        __debugbreak();
    hr = D3DCompileFromFile(L"./Shaders/DrawNormal.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain",
                            "ps_5_1", compileFlags, 0, &drawNormalPS, nullptr);
    if (FAILED(hr))
        __debugbreak();

    // Skinning CS
    hr = D3DCompileFromFile(L"./Shaders/SkinningCS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "cs_5_1",
                            compileFlags, 0, &deformingVertexCS, nullptr);
    if (FAILED(hr))
        __debugbreak();

    // Present
    hr = D3DCompileFromFile(L"./Shaders/PresentVS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1",
                            compileFlags, 0, &presentVS, nullptr);
    if (FAILED(hr))
        __debugbreak();

    hr = D3DCompileFromFile(L"./Shaders/PresentPS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1",
                            compileFlags, 0, &presentPS, nullptr);
    if (FAILED(hr))
        __debugbreak();
    /*hr = D3DCompileFromFile(L"./Shaders/D32ToRgbaPS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
    "ps_5_1", compileFlags, 0, &D32ToRgbaPS, nullptr); if (FAILED(hr))
        __debugbreak();*/

    hr = D3DCompileFromFile(L"./Shaders/DeferredPS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1",
                            compileFlags, 0, &deferredPS, nullptr);
    if (FAILED(hr))
        __debugbreak();

    hr = D3DCompileFromFile(L"./Shaders/SpriteDeferredPS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
                            "ps_5_1", compileFlags, 0, &spriteDeferredPS, nullptr);
    if (FAILED(hr))
        __debugbreak();

    hr = D3DCompileFromFile(L"./Shaders/LightOnlyPS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1",
                            compileFlags, 0, &lightOnlyPS, nullptr);
    if (FAILED(hr))
        __debugbreak();

    hr = D3DCompileFromFile(L"./Shaders/TransparencyPS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
                            "ps_5_1", compileFlags, 0, &transparencyPS, nullptr);

    if (FAILED(hr))
        __debugbreak();

    hr = D3DCompileFromFile(L"./Shaders/TransparentSpritePS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
                            "ps_5_1", compileFlags, 0, &transparentSpritePS, nullptr);

    if (FAILED(hr))
        __debugbreak();

    hr = D3DCompileFromFile(L"./Shaders/OITResolvePS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
                            "ps_5_1", compileFlags, 0, &OITReslovePS, nullptr);

    if (FAILED(hr))
        __debugbreak();

    // Tesselation
    {
        tessellatedQuadVS = CompileGraphicsShader(L"./Shaders/TessellatedQuad.hlsl", L"VSMain", L"vs_6_1", TRUE);
        tessellatedQuadHS = CompileGraphicsShader(L"./Shaders/TessellatedQuad.hlsl", L"HSMain", L"hs_6_1", TRUE);
        tessellatedQuadDS = CompileGraphicsShader(L"./Shaders/TessellatedQuad.hlsl", L"DSMain", L"ds_6_1", TRUE);
        tessellatedQuadPS = CompileGraphicsShader(L"./Shaders/BasicPS.hlsl", L"main", L"ps_6_1", TRUE);
        tessellatedQuadDeferredPS = CompileGraphicsShader(L"./Shaders/DeferredPS.hlsl", L"main", L"ps_6_1", TRUE);
    }

    // Terrain
    {
        terrainVS = CompileGraphicsShader(L"./Shaders/TerrainVS.hlsl", L"VSMainWithoutHS", L"vs_6_1", TRUE);
        terrainHS = CompileGraphicsShader(L"./Shaders/TerrainHS.hlsl", L"HSMain", L"hs_6_1", TRUE);
        terrainDS = CompileGraphicsShader(L"./Shaders/TerrainDS.hlsl", L"DSMain", L"ds_6_1", TRUE);
        terrainPS = CompileGraphicsShader(L"./Shaders/BasicPS.hlsl", L"main", L"ps_6_1", TRUE);
        terrainDeferredPS = CompileGraphicsShader(L"./Shaders/DeferredPS.hlsl", L"main", L"ps_6_1", TRUE);
    }

    // Mesh Object
    g_shaderData[RENDER_ITEM_TYPE_MESH_OBJ][DRAW_PASS_TYPE_DEFAULT] = {
        basicIL,
        CD3DX12_SHADER_BYTECODE(basicVS->GetBufferPointer(), basicVS->GetBufferSize()),
        CD3DX12_SHADER_BYTECODE(basicPS->GetBufferPointer(), basicPS->GetBufferSize()),
        {},
        {},
        {},
    };
    g_shaderData[RENDER_ITEM_TYPE_MESH_OBJ][DRAW_PASS_TYPE_SHADOW] = {
        basicIL,
        CD3DX12_SHADER_BYTECODE(depthOnlyVS->GetBufferPointer(), depthOnlyVS->GetBufferSize()),
        CD3DX12_SHADER_BYTECODE(depthOnlyPS->GetBufferPointer(), depthOnlyPS->GetBufferSize()),
        {},
        {},
        {},
    };

    g_shaderData[RENDER_ITEM_TYPE_MESH_OBJ][DRAW_PASS_TYPE_DEFERRED] = {
        basicIL,
        CD3DX12_SHADER_BYTECODE(basicVS->GetBufferPointer(), basicVS->GetBufferSize()),
        CD3DX12_SHADER_BYTECODE(deferredPS->GetBufferPointer(), deferredPS->GetBufferSize()),
        {},
        {},
        {},
    };
    g_shaderData[RENDER_ITEM_TYPE_MESH_OBJ][DRAW_PASS_TYPE_TRANSPARENCY] = {
        basicIL,
        CD3DX12_SHADER_BYTECODE(basicVS->GetBufferPointer(), basicVS->GetBufferSize()),
        CD3DX12_SHADER_BYTECODE(transparencyPS->GetBufferPointer(), transparencyPS->GetBufferSize()),
        {},
        {},
        {},
    };

    // Skinned Object
    g_shaderData[RENDER_ITEM_TYPE_CHAR_OBJ][DRAW_PASS_TYPE_DEFAULT] = {
        skinnedIL,
        CD3DX12_SHADER_BYTECODE(basicSkinnedVS->GetBufferPointer(), basicSkinnedVS->GetBufferSize()),
        CD3DX12_SHADER_BYTECODE(basicPS->GetBufferPointer(), basicPS->GetBufferSize()),
        {},
        {},
        {},
    };
    g_shaderData[RENDER_ITEM_TYPE_CHAR_OBJ][DRAW_PASS_TYPE_SHADOW] = {
        skinnedIL,
        CD3DX12_SHADER_BYTECODE(depthOnlySkinnedVS->GetBufferPointer(), depthOnlySkinnedVS->GetBufferSize()),
        CD3DX12_SHADER_BYTECODE(depthOnlyPS->GetBufferPointer(), depthOnlyPS->GetBufferSize()),
        {},
        {},
        {},
    };
    g_shaderData[RENDER_ITEM_TYPE_CHAR_OBJ][DRAW_PASS_TYPE_DEFERRED] = {
        skinnedIL,
        CD3DX12_SHADER_BYTECODE(basicSkinnedVS->GetBufferPointer(), basicSkinnedVS->GetBufferSize()),
        CD3DX12_SHADER_BYTECODE(deferredPS->GetBufferPointer(), deferredPS->GetBufferSize()),
        {},
        {},
        {},
    };
    g_shaderData[RENDER_ITEM_TYPE_CHAR_OBJ][DRAW_PASS_TYPE_TRANSPARENCY] = {
        skinnedIL,
        CD3DX12_SHADER_BYTECODE(basicSkinnedVS->GetBufferPointer(), basicSkinnedVS->GetBufferSize()),
        CD3DX12_SHADER_BYTECODE(transparencyPS->GetBufferPointer(), transparencyPS->GetBufferSize()),
        {},
        {},
        {},
    };

    // Sprite
    g_shaderData[RENDER_ITEM_TYPE_SPRITE][DRAW_PASS_TYPE_DEFAULT] = {
        simpleIL,
        CD3DX12_SHADER_BYTECODE(spriteVS->GetBufferPointer(), spriteVS->GetBufferSize()),
        CD3DX12_SHADER_BYTECODE(spritePS->GetBufferPointer(), spritePS->GetBufferSize()),
        {},
        {},
        {},
    };
    g_shaderData[RENDER_ITEM_TYPE_SPRITE][DRAW_PASS_TYPE_SHADOW] = {};
    g_shaderData[RENDER_ITEM_TYPE_SPRITE][DRAW_PASS_TYPE_DEFERRED] = {
        simpleIL,
        CD3DX12_SHADER_BYTECODE(spriteVS->GetBufferPointer(), spriteVS->GetBufferSize()),
        CD3DX12_SHADER_BYTECODE(spriteDeferredPS->GetBufferPointer(), spriteDeferredPS->GetBufferSize()),
        {},
        {},
        {},
    };

    g_shaderData[RENDER_ITEM_TYPE_SPRITE][DRAW_PASS_TYPE_TRANSPARENCY] = {
        simpleIL,
        CD3DX12_SHADER_BYTECODE(spriteVS->GetBufferPointer(), spriteVS->GetBufferSize()),
        CD3DX12_SHADER_BYTECODE(transparentSpritePS->GetBufferPointer(), transparentSpritePS->GetBufferSize()),
        {},
        {},
        {},
    };

    // Tessellation
    /*g_shaderData[RENDER_ITEM_TYPE_TERRAIN][DRAW_PASS_TYPE_DEFAULT] = {
        terrainIL,
        CD3DX12_SHADER_BYTECODE(terrainVS->GetBufferPointer(), terrainVS->GetBufferSize()),
        CD3DX12_SHADER_BYTECODE(terrainPS->GetBufferPointer(), terrainPS->GetBufferSize()),
        CD3DX12_SHADER_BYTECODE(terrainHS->GetBufferPointer(), terrainHS->GetBufferSize()),
        CD3DX12_SHADER_BYTECODE(terrainDS->GetBufferPointer(), terrainDS->GetBufferSize()),
        {},
    };
    g_shaderData[RENDER_ITEM_TYPE_TERRAIN][DRAW_PASS_TYPE_DEFERRED] = {
        terrainIL,
        CD3DX12_SHADER_BYTECODE(terrainVS->GetBufferPointer(), terrainVS->GetBufferSize()),
        CD3DX12_SHADER_BYTECODE(terrainDeferredPS->GetBufferPointer(), terrainDeferredPS->GetBufferSize()),
        CD3DX12_SHADER_BYTECODE(terrainHS->GetBufferPointer(), terrainHS->GetBufferSize()),
        CD3DX12_SHADER_BYTECODE(terrainDS->GetBufferPointer(), terrainDS->GetBufferSize()),
        {},
    };*/
    g_shaderData[RENDER_ITEM_TYPE_TERRAIN][DRAW_PASS_TYPE_DEFAULT] = {
        terrainIL,
        CD3DX12_SHADER_BYTECODE(terrainVS->GetBufferPointer(), terrainVS->GetBufferSize()),
        CD3DX12_SHADER_BYTECODE(terrainPS->GetBufferPointer(), terrainPS->GetBufferSize()),
        {},
        {},
        {},
    };
    g_shaderData[RENDER_ITEM_TYPE_TERRAIN][DRAW_PASS_TYPE_DEFERRED] = {
        terrainIL,
        CD3DX12_SHADER_BYTECODE(terrainVS->GetBufferPointer(), terrainVS->GetBufferSize()),
        CD3DX12_SHADER_BYTECODE(terrainDeferredPS->GetBufferPointer(), terrainDeferredPS->GetBufferSize()),
        {},
        {},
        {},
    };

    g_additionalShaderData[ADDITIONAL_PIPELINE_TYPE_POST_PROCESS] = {
        simpleIL,
        CD3DX12_SHADER_BYTECODE(presentVS->GetBufferPointer(), presentVS->GetBufferSize()),
        CD3DX12_SHADER_BYTECODE(presentPS->GetBufferPointer(), presentPS->GetBufferSize()),
        {},
        {},
        {},
    };
    g_additionalShaderData[ADDITIONAL_PIPELINE_TYPE_OIT_RESOLVE] = {
        simpleIL,
        CD3DX12_SHADER_BYTECODE(presentVS->GetBufferPointer(), presentVS->GetBufferSize()),
        CD3DX12_SHADER_BYTECODE(OITReslovePS->GetBufferPointer(), OITReslovePS->GetBufferSize()),
        {},
        {},
        {},
    };
}

void Graphics::InitSamplers()
{
    D3D12_STATIC_SAMPLER_DESC desc;
    desc.MipLODBias = 0;
    desc.MaxAnisotropy = D3D12_MAX_MAXANISOTROPY;
    desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    desc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
    desc.MinLOD = 0;
    desc.MaxLOD = D3D12_FLOAT32_MAX;
    desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    desc.RegisterSpace = 0;

    // LinearWrap
    desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    desc.ShaderRegister = SAMPLER_TYPE_LINEAR_WRAP;
    samplerStates[SAMPLER_TYPE_LINEAR_WRAP] = desc;

    // LinearClamp
    desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    desc.ShaderRegister = SAMPLER_TYPE_LINEAR_CLAMP;
    samplerStates[SAMPLER_TYPE_LINEAR_CLAMP] = desc;

    // PointWrap
    desc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    desc.ShaderRegister = SAMPLER_TYPE_POINT_WRAP;
    samplerStates[SAMPLER_TYPE_POINT_WRAP] = desc;

    // PointClamp
    desc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    desc.ShaderRegister = SAMPLER_TYPE_POINT_CLAMP;
    samplerStates[SAMPLER_TYPE_POINT_CLAMP] = desc;

    // shadowPoint
    desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    desc.ShaderRegister = SAMPLER_TYPE_SHADOW_POINT;
    desc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    samplerStates[SAMPLER_TYPE_SHADOW_POINT] = desc;

    // AnisotropicWrap
    desc.Filter = D3D12_FILTER_ANISOTROPIC;
    desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    desc.ShaderRegister = SAMPLER_TYPE_ANISOTROPIC_WRAP;
    samplerStates[SAMPLER_TYPE_ANISOTROPIC_WRAP] = desc;

    // AnisotropicClamp
    desc.Filter = D3D12_FILTER_ANISOTROPIC;
    desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    desc.ShaderRegister = SAMPLER_TYPE_ANISOTROPIC_CLAMP;
    samplerStates[SAMPLER_TYPE_ANISOTROPIC_CLAMP] = desc;
}

void Graphics::InitRootSignature(ID3D12Device5 *pD3DDevice)
{
    HRESULT                           hr = S_OK;
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

    // This is the highest version the sample supports. If CheckFeatureSupport
    // succeeds, the HighestVersion returned will not be greater than this.
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

    hr = pD3DDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData));
    if (FAILED(hr))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    // Create an empty root signature.
    {
        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(0, nullptr, 0, nullptr,
                                   D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        SerializeAndCreateVersionedRootSignature(pD3DDevice, &rootSignatureDesc, featureData.HighestVersion, &emptyRS,
                                                 L"EmptyRS");
    }

    // Ranges Per Global
    // | Global Consts(b0) |
    // | Materials (t20)   |
    // | EnvIBL(t10)       |
    CD3DX12_DESCRIPTOR_RANGE1 rangesPerGlobal[3] = {};
    rangesPerGlobal[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);  // b0 : globalConsts
    rangesPerGlobal[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 20); // t20 : materials
    rangesPerGlobal[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 10); // t10 : EnvIBL

    // Ranges Per Basic Object  
    // |  TR Matrix(b1)   |
    CD3DX12_DESCRIPTOR_RANGE1 rangesPerBasicObj[1] = {};
    rangesPerBasicObj[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

    // Ranges Per Skinned Object
    // |  TR Matrix(b1)   |  Bone Matrix |
    CD3DX12_DESCRIPTOR_RANGE1 rangesPerSkinnedObj[1] = {};
    rangesPerSkinnedObj[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 1);

    // Ranges Per Material
    // |   Geometry(b5)   |  albedoTex(t0) | normalTex(t1) | aoTex(t2) | metallicRoughnesesTex(t3) | emissiveTex(t4) | heightTex(t5) | 
    CD3DX12_DESCRIPTOR_RANGE1 rangesPerMaterial[2] = {};
    rangesPerMaterial[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 5);
    rangesPerMaterial[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 6, 0);

    // Ranges Per Sprite
    CD3DX12_DESCRIPTOR_RANGE1 rangesPerSprite[2] = {};
    rangesPerSprite[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // b0 : Constant Buffer View
    rangesPerSprite[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0 : Shader Resource View(Tex)

    // Ranges Per OIT Fragment List
    // |  FRAGMENTLISTFIRSTNODEADDRESS UAV | FRAGMENTLIST UAV |  FRAGMENTLISTFIRSTNODEADDRESS SRV | FRAGMENTLIST SRV | 
    CD3DX12_DESCRIPTOR_RANGE1 rangesPerFragmentList[2] = {};
    rangesPerFragmentList[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 2, 0); // u0~u1
    rangesPerFragmentList[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 18); // t18~19
    
    // Init Basic Root Signature
    {
        // | Ranges Per Global          |
        // | Ranges Per Basic Object    |
        // | Ranges Per Material        |
        CD3DX12_ROOT_PARAMETER1   rootParameters[3] = {};
        rootParameters[0].InitAsDescriptorTable(_countof(rangesPerGlobal), &rangesPerGlobal[0],
                                                D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[1].InitAsDescriptorTable(_countof(rangesPerBasicObj), &rangesPerBasicObj[0],
                                                D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[2].InitAsDescriptorTable(_countof(rangesPerMaterial), &rangesPerMaterial[0],
                                                D3D12_SHADER_VISIBILITY_ALL);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, _countof(samplerStates), samplerStates,
                                   D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        SerializeAndCreateVersionedRootSignature(pD3DDevice, &rootSignatureDesc, featureData.HighestVersion, &basicRS,
                                                 L"BasicRS");
    }

    // Init Skinned Root Signature
    {
        // | Ranges Per Global          |
        // | Ranges Per Skinned Object  |
        // | Ranges Per Material        |
        CD3DX12_ROOT_PARAMETER1 rootParameters[3] = {};
        rootParameters[0].InitAsDescriptorTable(_countof(rangesPerGlobal), &rangesPerGlobal[0],
                                                D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[1].InitAsDescriptorTable(_countof(rangesPerSkinnedObj), &rangesPerSkinnedObj[0],
                                                D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[2].InitAsDescriptorTable(_countof(rangesPerMaterial), &rangesPerMaterial[0],
                                                D3D12_SHADER_VISIBILITY_ALL);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, SAMPLER_TYPE_COUNT, samplerStates,
                                   D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        SerializeAndCreateVersionedRootSignature(pD3DDevice, &rootSignatureDesc, featureData.HighestVersion, &skinnedRS,
                                                 L"SkinnedRS");
    }

    // Init Skybox Root Signature
    {

        CD3DX12_ROOT_PARAMETER1 rootParameters[1] = {};
        rootParameters[0].InitAsDescriptorTable(_countof(rangesPerGlobal), &rangesPerGlobal[0],
                                                D3D12_SHADER_VISIBILITY_ALL);
        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, SAMPLER_TYPE_COUNT, samplerStates,
                                   D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        SerializeAndCreateVersionedRootSignature(pD3DDevice, &rootSignatureDesc, featureData.HighestVersion, &skyboxRS,
                                                 L"SkyboxRS");
    }

    // Init Sprite Root Signature
    {
        CD3DX12_ROOT_PARAMETER1 rootParameters[1] = {};
        rootParameters[0].InitAsDescriptorTable(_countof(rangesPerSprite), rangesPerSprite,
                                                D3D12_SHADER_VISIBILITY_ALL);

        // Create an empty root signature.
        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, SAMPLER_TYPE_COUNT, samplerStates,
                               D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        SerializeAndCreateVersionedRootSignature(pD3DDevice, &rootSignatureDesc, featureData.HighestVersion, &spriteRS,
                                                 L"SpriteRS");
    }

    // Init Skinning(Deforming Vertex) Root Signature
    {
        CD3DX12_DESCRIPTOR_RANGE ranges[3] = {};
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); // u0
        ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // b0

        CD3DX12_ROOT_PARAMETER rootParameters[1] = {};
        rootParameters[0].InitAsDescriptorTable(_countof(ranges), ranges, D3D12_SHADER_VISIBILITY_ALL);
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
        SerializeAndCreateRootSignature(pD3DDevice, &rootSignatureDesc, &deformingVertexRS, L"deformingVertex RS");
    }

    // Init Present Root Signature
    {
        CD3DX12_DESCRIPTOR_RANGE ranges[1] = {};
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0

        CD3DX12_ROOT_PARAMETER rootParameters[1] = {};

        rootParameters[0].InitAsDescriptorTable(_countof(ranges), ranges, D3D12_SHADER_VISIBILITY_ALL);

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters, 1, samplerStates,
                                                      D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        SerializeAndCreateRootSignature(pD3DDevice, &rootSignatureDesc, &presentRS, L"present RS");
    }

    // Init DepthOnly Basic Root Signature
    {
        // Root Paramaters
        // | Ranges Per Global |
        // | Ranges Per Basic Object |
        CD3DX12_ROOT_PARAMETER1 rootParameters[2] = {};
        rootParameters[0].InitAsDescriptorTable(1, &rangesPerGlobal[0], D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[1].InitAsDescriptorTable(_countof(rangesPerBasicObj), rangesPerBasicObj,
                                                D3D12_SHADER_VISIBILITY_ALL);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc(
            ARRAYSIZE(rootParameters), rootParameters, SAMPLER_TYPE_COUNT, samplerStates,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        SerializeAndCreateVersionedRootSignature(pD3DDevice, &rootSignatureDesc, featureData.HighestVersion,
                                                 &depthOnlyBasicRS, L"DepthOnly Basic RS");
    }
    
    // Init DepthOnly Skinned Root Signature
    {
        // Root Paramaters
        // | Ranges Per Global |
        // | Ranges Per Skinned Object |
        CD3DX12_ROOT_PARAMETER1 rootParameters[2] = {};
        rootParameters[0].InitAsDescriptorTable(1, &rangesPerGlobal[0], D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[1].InitAsDescriptorTable(_countof(rangesPerSkinnedObj), rangesPerSkinnedObj,
                                                D3D12_SHADER_VISIBILITY_ALL);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc(
            ARRAYSIZE(rootParameters), rootParameters, SAMPLER_TYPE_COUNT, samplerStates,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        SerializeAndCreateVersionedRootSignature(pD3DDevice, &rootSignatureDesc, featureData.HighestVersion,
                                                 &depthOnlySkinnedRS, L"DepthOnly Skinned RS");
    }

    // Init Transparent Basic Root Signature
    {
        // | Ranges Per Global            |
        // | Ranges Per Basic Object      |
        // | Ranges Per Material          |
        // | Ranges Per OIT Fragment List |
        CD3DX12_ROOT_PARAMETER1 rootParameters[4] = {};
        rootParameters[0].InitAsDescriptorTable(_countof(rangesPerGlobal), &rangesPerGlobal[0],
                                                D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[1].InitAsDescriptorTable(_countof(rangesPerBasicObj), &rangesPerBasicObj[0],
                                                D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[2].InitAsDescriptorTable(_countof(rangesPerMaterial), &rangesPerMaterial[0],
                                                D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[3].InitAsDescriptorTable(_countof(rangesPerFragmentList), &rangesPerFragmentList[0],
                                                D3D12_SHADER_VISIBILITY_ALL);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, _countof(samplerStates), samplerStates,
                                   D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        SerializeAndCreateVersionedRootSignature(pD3DDevice, &rootSignatureDesc, featureData.HighestVersion,
                                                 &transparentBasicRS, L"transparentBasicRS");
    }

    // Init transparent Skinned Root Signature
    {
        // | Ranges Per Global          |
        // | Ranges Per Skinned Object  |
        // | Ranges Per Material        |
        // | Ranges Per OIT Fragment List |
        CD3DX12_ROOT_PARAMETER1 rootParameters[4] = {};
        rootParameters[0].InitAsDescriptorTable(_countof(rangesPerGlobal), &rangesPerGlobal[0],
                                                D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[1].InitAsDescriptorTable(_countof(rangesPerSkinnedObj), &rangesPerSkinnedObj[0],
                                                D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[2].InitAsDescriptorTable(_countof(rangesPerMaterial), &rangesPerMaterial[0],
                                                D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[3].InitAsDescriptorTable(_countof(rangesPerFragmentList), &rangesPerFragmentList[0],
                                                D3D12_SHADER_VISIBILITY_ALL);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, SAMPLER_TYPE_COUNT, samplerStates,
                                   D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        SerializeAndCreateVersionedRootSignature(pD3DDevice, &rootSignatureDesc, featureData.HighestVersion,
                                                 &transparentSkinnedRS, L"transparentSkinnedRS");
    }

    // Init transparent Sprite Root Signature
    {
        // | Ranges Per Sprite            |
        // | Ranges Per OIT Fragment List |
        CD3DX12_ROOT_PARAMETER1 rootParameters[2] = {};
        rootParameters[0].InitAsDescriptorTable(_countof(rangesPerSprite), rangesPerSprite,
                                                D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[1].InitAsDescriptorTable(_countof(rangesPerFragmentList), &rangesPerFragmentList[0],
                                                D3D12_SHADER_VISIBILITY_ALL);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, SAMPLER_TYPE_COUNT, samplerStates,
                                   D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        SerializeAndCreateVersionedRootSignature(pD3DDevice, &rootSignatureDesc, featureData.HighestVersion,
                                                 &transparentSpriteRS, L"transparentSpriteRS");
    }

    // Init Second Pass Root Signature
    {
        // Root Paramaters
        // | ranges per Global |
        // |   DiffuseTex   | NormalTex | ElementsTex | DepthTex |
        CD3DX12_DESCRIPTOR_RANGE1 ranges[1] = {};
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0); // t0~t3

        CD3DX12_ROOT_PARAMETER1 rootParameters[2] = {};

        rootParameters[0].InitAsDescriptorTable(_countof(rangesPerGlobal), rangesPerGlobal,
                                                D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[1].InitAsDescriptorTable(_countof(ranges), ranges, D3D12_SHADER_VISIBILITY_ALL);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(ARRAYSIZE(rootParameters), rootParameters, SAMPLER_TYPE_COUNT, samplerStates,
                                   D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        SerializeAndCreateVersionedRootSignature(pD3DDevice, &rootSignatureDesc, featureData.HighestVersion,
                                                 &secondPassRS, L"SecondPassRS");
    }

    // Init OIT Resolve Root Signature
    {
        // Root Paramaters
        // | ranges per OIT FragmentList |
        CD3DX12_ROOT_PARAMETER1 rootParameters[1] = {};

        rootParameters[0].InitAsDescriptorTable(_countof(rangesPerFragmentList), rangesPerFragmentList,
                                                D3D12_SHADER_VISIBILITY_ALL);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(ARRAYSIZE(rootParameters), rootParameters, 0, nullptr,
                                   D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        SerializeAndCreateVersionedRootSignature(pD3DDevice, &rootSignatureDesc, featureData.HighestVersion,
                                                 &OITResolveRS, L"OITResolveRS");
    }

    rootSignatures[RENDER_ITEM_TYPE_MESH_OBJ][DRAW_PASS_TYPE_DEFERRED] = basicRS;

    rootSignatures[RENDER_ITEM_TYPE_CHAR_OBJ][DRAW_PASS_TYPE_DEFERRED] = skinnedRS;

    rootSignatures[RENDER_ITEM_TYPE_SPRITE][DRAW_PASS_TYPE_DEFERRED] = spriteRS;

    rootSignatures[RENDER_ITEM_TYPE_TERRAIN][DRAW_PASS_TYPE_DEFERRED] = basicRS;

    rootSignatures[RENDER_ITEM_TYPE_MESH_OBJ][DRAW_PASS_TYPE_TRANSPARENCY] = transparentBasicRS;

    rootSignatures[RENDER_ITEM_TYPE_CHAR_OBJ][DRAW_PASS_TYPE_TRANSPARENCY] = transparentSkinnedRS;

    rootSignatures[RENDER_ITEM_TYPE_SPRITE][DRAW_PASS_TYPE_TRANSPARENCY] = transparentSpriteRS;
}

void Graphics::InitPipelineStates(ID3D12Device5 *pD3DDevice)
{
    HRESULT hr = S_OK;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

    for (UINT itemType = 0; itemType < RENDER_ITEM_TYPE_COUNT; itemType++)
    {
        for (UINT passType = 0; passType < DRAW_PASS_TYPE_COUNT; passType++)
        {
            if (!rootSignatures[itemType][passType])
                continue;
            switch (passType)
            {
            case DRAW_PASS_TYPE_DEFERRED:
                psoDesc.NumRenderTargets = 3;
                psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
                psoDesc.RTVFormats[1] = DXGI_FORMAT_R16G16B16A16_FLOAT;
                psoDesc.RTVFormats[2] = DXGI_FORMAT_R16G16B16A16_FLOAT;
                break;
            case DRAW_PASS_TYPE_TRANSPARENCY:
                psoDesc.NumRenderTargets = 0;
                psoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
                psoDesc.RTVFormats[1] = DXGI_FORMAT_UNKNOWN;
                psoDesc.RTVFormats[2] = DXGI_FORMAT_UNKNOWN;
                break;
            default:
                psoDesc.NumRenderTargets = 1;
                psoDesc.RTVFormats[1] = DXGI_FORMAT_UNKNOWN;
                psoDesc.RTVFormats[2] = DXGI_FORMAT_UNKNOWN;
                break;
            }

            psoDesc.InputLayout = g_shaderData[itemType][passType].inputLayout;
            psoDesc.pRootSignature = rootSignatures[itemType][passType];
            psoDesc.VS = g_shaderData[itemType][passType].VS;
            psoDesc.PS = g_shaderData[itemType][passType].PS;
            psoDesc.DS = g_shaderData[itemType][passType].DS;
            psoDesc.HS = g_shaderData[itemType][passType].HS;
            psoDesc.GS = g_shaderData[itemType][passType].GS;

            for (UINT fillMode = 0; fillMode < FILL_MODE_COUNT; fillMode++)
            {
                if (fillMode == FILL_MODE_SOLID)
                {
                    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
                }
                else if (fillMode == FILL_MODE_WIRED)
                {
                    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
                    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
                }

                hr =
                    pD3DDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&PSO[itemType][passType][fillMode]));
                if (FAILED(hr))
                {
                    __debugbreak();
                }
            }
        }
    }

    // present PSO
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature = presentRS;
        psoDesc.InputLayout = g_additionalShaderData[ADDITIONAL_PIPELINE_TYPE_POST_PROCESS].inputLayout;
        psoDesc.VS = g_additionalShaderData[ADDITIONAL_PIPELINE_TYPE_POST_PROCESS].VS;
        psoDesc.PS = g_additionalShaderData[ADDITIONAL_PIPELINE_TYPE_POST_PROCESS].PS;

        psoDesc.SampleMask = UINT_MAX;
        psoDesc.SampleDesc.Count = 1;
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;

        if (FAILED(pD3DDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&presentPSO))))
        {
            __debugbreak();
        }
    }

    // deforming Vertex PSO
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
        desc.CS = CD3DX12_SHADER_BYTECODE(deformingVertexCS->GetBufferPointer(), deformingVertexCS->GetBufferSize());
        desc.pRootSignature = deformingVertexRS;

        if (FAILED(pD3DDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(&deformingVertexPSO))))
        {
            __debugbreak();
        }
    }
}

HRESULT Graphics::InitRaytracingStates(ID3D12Device5 *pD3DDevice)
{
    HRESULT hr = S_OK;

    CD3DX12_STATE_OBJECT_DESC raytracingPipeline{D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE};
    InitRaytracingShaders(&raytracingPipeline);
    InitRaytracingRootSignatures(pD3DDevice);
    InitRaytracingStateObjects(&raytracingPipeline);

    hr = pD3DDevice->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&rtStateObject));
    if (FAILED(hr))
    {
        __debugbreak();
        goto lb_return;
    }
    hr = rtStateObject->QueryInterface(IID_PPV_ARGS(&rtStateObjectProps));
    if (FAILED(hr))
    {
        __debugbreak();
        goto lb_return;
    }

lb_return:
    return hr;
}

void Graphics::InitRaytracingShaders(CD3DX12_STATE_OBJECT_DESC *raytracingPipeline)
{
    BOOL disableOptimize = FALSE;
#ifdef _DEBUG
    disableOptimize = TRUE;
#endif // _DEBUG

#ifdef USE_DEFERRED_RENDERING
    DxcDefine defines[] = {{L"DEFERRED", L"1"}};
    raytracingLibrary =
        CompileShaderLibrary(L"./Shaders/DXR/Raytracing.hlsl", disableOptimize, defines, _countof(defines));
#else
    raytracingLibrary = CompileShaderLibrary(L"./Shaders/DXR/Raytracing.hlsl", disableOptimize, nullptr, 0);
#endif // USE_DEFERRED_RENDERING

    auto lib = raytracingPipeline->CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    lib->SetDXILLibrary(
        &CD3DX12_SHADER_BYTECODE(raytracingLibrary->GetBufferPointer(), raytracingLibrary->GetBufferSize()));
    //
    // #ifdef USE_DEFERRED_RENDERING
    //     rayGenLibrary = CompileShaderLibrary(L"./Shaders/DXR/DeferredRaygen.hlsl", disableOptimize);
    //     auto lib = raytracingPipeline->CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    //     lib->SetDXILLibrary(&CD3DX12_SHADER_BYTECODE(rayGenLibrary->GetBufferPointer(),
    //     rayGenLibrary->GetBufferSize()));
    // #else
    //     rayGenLibrary = CompileShaderLibrary(L"./Shaders/DXR/RayGen.hlsl", disableOptimize);
    //     auto lib = raytracingPipeline->CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    //     lib->SetDXILLibrary(&CD3DX12_SHADER_BYTECODE(rayGenLibrary->GetBufferPointer(),
    //     rayGenLibrary->GetBufferSize()));
    // #endif
    //
    //     missLibrary = CompileShaderLibrary(L"./Shaders/DXR/Miss.hlsl", disableOptimize);
    //     lib = raytracingPipeline->CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    //     lib->SetDXILLibrary(&CD3DX12_SHADER_BYTECODE(missLibrary->GetBufferPointer(), missLibrary->GetBufferSize()));
    //
    //     hitLibrary = CompileShaderLibrary(L"./Shaders/DXR/Hit.hlsl", disableOptimize);
    //     lib = raytracingPipeline->CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    //     lib->SetDXILLibrary(&CD3DX12_SHADER_BYTECODE(hitLibrary->GetBufferPointer(), hitLibrary->GetBufferSize()));
    //
    //     shadowLibrary = CompileShaderLibrary(L"./Shaders/DXR/ShadowRay.hlsl", disableOptimize);
    //     lib = raytracingPipeline->CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    //     lib->SetDXILLibrary(&CD3DX12_SHADER_BYTECODE(shadowLibrary->GetBufferPointer(),
    //     shadowLibrary->GetBufferSize()));
}

void Graphics::InitRaytracingRootSignatures(ID3D12Device5 *pD3DDevice)
{
    // Init Global Root Signature
    // |   OUTPUT_VIEW(u0)   | ACCELERATION_STRUCTURE(t0)  | DIFFUSE ! NORMAL ! ELEMENTS ! DEPTH ! // !:deferred
    // | GlobalConstants(b0) |       Materials(t20)        |   EnvIBL(t10)    |
    {
        CD3DX12_DESCRIPTOR_RANGE rangesPerGlobal1[2] = {};
        rangesPerGlobal1[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); // u0
#ifdef USE_DEFERRED_RENDERING
        rangesPerGlobal1[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0); // t0~t4
#else
        rangesPerGlobal1[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0
#endif

        CD3DX12_DESCRIPTOR_RANGE rangesPerGlobal2[3] = {};
        rangesPerGlobal2[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
        rangesPerGlobal2[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 20);
        rangesPerGlobal2[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 10);

        CD3DX12_ROOT_PARAMETER rootParameters[2] = {};
        rootParameters[0].InitAsDescriptorTable(ARRAYSIZE(rangesPerGlobal1), rangesPerGlobal1);
        rootParameters[1].InitAsDescriptorTable(ARRAYSIZE(rangesPerGlobal2), rangesPerGlobal2);

        CD3DX12_STATIC_SAMPLER_DESC staticSamplers[] = {
            // LinearWrapSampler
            CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_ANISOTROPIC),
        };
        CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters,
                                                            ARRAYSIZE(staticSamplers), staticSamplers);
        SerializeAndCreateRootSignature(pD3DDevice, &globalRootSignatureDesc, &globalRS, L"GlbalRootSig");
    }

    // Init Local Hit Root Signature
    // | VERTICES(t0, space1) | INDICES(t1, space1) | ALBEDO_TEX(t2, space1) | NORMAL_TEX(t3, space1) | AO_TEX(t4,
    // space1) | METALLIC_ROUGHNESS(t5, space1) | EMISSIVE(t6, space1) | Height(t7, space1) |
    {
        CD3DX12_DESCRIPTOR_RANGE ranges[LOCAL_ROOT_PARAM_INDEX_COUNT] = {};
        ranges[LOCAL_ROOT_PARAM_INDEX_VERTICES].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);
        ranges[LOCAL_ROOT_PARAM_INDEX_INDICES].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 1);
        ranges[LOCAL_ROOT_PARAM_INDEX_TEXTURES].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 6, 2, 1);

        CD3DX12_ROOT_PARAMETER rootParameters[LOCAL_ROOT_PARAM_INDEX_COUNT];
        rootParameters[LOCAL_ROOT_PARAM_INDEX_CB].InitAsConstants(SizeOfInUint32(RaytracingFaceGroupCB), 0, 1);
        rootParameters[LOCAL_ROOT_PARAM_INDEX_VERTICES].InitAsDescriptorTable(1,
                                                                              &ranges[LOCAL_ROOT_PARAM_INDEX_VERTICES]);
        rootParameters[LOCAL_ROOT_PARAM_INDEX_INDICES].InitAsDescriptorTable(1,
                                                                             &ranges[LOCAL_ROOT_PARAM_INDEX_INDICES]);
        rootParameters[LOCAL_ROOT_PARAM_INDEX_TEXTURES].InitAsDescriptorTable(1,
                                                                              &ranges[LOCAL_ROOT_PARAM_INDEX_TEXTURES]);

        CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
        localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;

        SerializeAndCreateRootSignature(pD3DDevice, &localRootSignatureDesc, &localHitRS, L"LocalRootSig");
    }

    // Init Local Empty Root Signature
    {
        // Create an empty root signature.
        CD3DX12_ROOT_SIGNATURE_DESC emptyRootSignatureDesc;
        emptyRootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
        SerializeAndCreateRootSignature(pD3DDevice, &emptyRootSignatureDesc, &localEmptyRS, L"LocalEmptySig");
    }
}

void Graphics::InitRaytracingStateObjects(CD3DX12_STATE_OBJECT_DESC *raytracingPipeline)
{
    // Create Hit Group
    auto hitGroup = raytracingPipeline->CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    hitGroup->SetClosestHitShaderImport(hitShaderNames[0]);
    hitGroup->SetHitGroupExport(hitGroupNames[0]);
    hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

    // Create Shadow Hit Group
    auto shadowHitGroup = raytracingPipeline->CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    shadowHitGroup->SetClosestHitShaderImport(hitShaderNames[1]);
    shadowHitGroup->SetHitGroupExport(hitGroupNames[1]);
    shadowHitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

    // Shader config
    // Defines the maximum sizes in bytes for the ray rayPayload and attribute structure.
    auto shaderConfig = raytracingPipeline->CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    UINT payloadSize = static_cast<UINT>(4 * sizeof(float) + sizeof(UINT)); // RecursionDepth + RGB + distance
    UINT attributeSize = 2 * sizeof(float);                                 // float2 barycentrics
    shaderConfig->Config(payloadSize, attributeSize);

    auto globalRootSignature = raytracingPipeline->CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
    globalRootSignature->SetRootSignature(globalRS);

    auto localRootSignature = raytracingPipeline->CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
    localRootSignature->SetRootSignature(localHitRS);
    // Shader association
    auto rootSignatureAssociation =
        raytracingPipeline->CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
    rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
    rootSignatureAssociation->AddExports(hitGroupNames);

    // Pipeline config
    // Defines the maximum TraceRay() recursion depth.
    auto pipelineConfig = raytracingPipeline->CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
    // PERFOMANCE TIP: Set max recursion depth as low as needed
    // as drivers may apply optimization strategies for low recursion depths.
    UINT maxRecursionDepth = 5;
    pipelineConfig->Config(maxRecursionDepth);
}

ID3D12RootSignature *Graphics::GetRS(RENDER_ITEM_TYPE itemType, DRAW_PASS_TYPE passType)
{
    return rootSignatures[itemType][passType];
}

ID3D12PipelineState *Graphics::GetPSO(RENDER_ITEM_TYPE itemType, DRAW_PASS_TYPE passType, FILL_MODE fillMode)
{
    return PSO[itemType][passType][fillMode];
}

void Graphics::DeleteCommonStates()
{
    DeleteShaders();
    DeleteRootSignatures();
    DeletePipelineStates();
    DeleteRaytracingStates();
}

void Graphics::DeleteShaders()
{
    if (basicVS)
    {
        basicVS->Release();
        basicVS = nullptr;
    }
    if (basicSkinnedVS)
    {
        basicSkinnedVS->Release();
        basicSkinnedVS = nullptr;
    }
    if (basicPS)
    {
        basicPS->Release();
        basicPS = nullptr;
    }
    if (depthOnlyVS)
    {
        depthOnlyVS->Release();
        depthOnlyVS = nullptr;
    }
    if (depthOnlySkinnedVS)
    {
        depthOnlySkinnedVS->Release();
        depthOnlySkinnedVS = nullptr;
    }
    if (depthOnlyPS)
    {
        depthOnlyPS->Release();
        depthOnlyPS = nullptr;
    }
    if (spriteVS)
    {
        spriteVS->Release();
        spriteVS = nullptr;
    }
    if (spritePS)
    {
        spritePS->Release();
        spritePS = nullptr;
    }
    if (deferredPS)
    {
        deferredPS->Release();
        deferredPS = nullptr;
    }
    if (spriteDeferredPS)
    {
        spriteDeferredPS->Release();
        spriteDeferredPS = nullptr;
    }
    if (skyboxVS)
    {
        skyboxVS->Release();
        skyboxVS = nullptr;
    }
    if (skyboxPS)
    {
        skyboxPS->Release();
        skyboxPS = nullptr;
    }
    if (drawNormalVS)
    {
        drawNormalVS->Release();
        drawNormalVS = nullptr;
    }
    if (drawNormalSkinnedVS)
    {
        drawNormalSkinnedVS->Release();
        drawNormalSkinnedVS = nullptr;
    }
    if (drawNormalGS)
    {
        drawNormalGS->Release();
        drawNormalGS = nullptr;
    }
    if (drawNormalPS)
    {
        drawNormalPS->Release();
        drawNormalPS = nullptr;
    }
    if (presentVS)
    {
        presentVS->Release();
        presentVS = nullptr;
    }
    if (presentPS)
    {
        presentPS->Release();
        presentPS = nullptr;
    }
    if (lightOnlyPS)
    {
        lightOnlyPS->Release();
        lightOnlyPS = nullptr;
    }
    if (transparencyPS)
    {
        transparencyPS->Release();
        transparencyPS = nullptr;
    }
    if (transparentSpritePS)
    {
        transparentSpritePS->Release();
        transparentSpritePS = nullptr;
    }
}

void Graphics::DeleteSamplers() {}

void Graphics::DeleteRootSignatures()
{
    if (emptyRS)
    {
        emptyRS->Release();
        emptyRS = nullptr;
    }
    if (deformingVertexRS)
    {
        deformingVertexRS->Release();
        deformingVertexRS = nullptr;
    }
    if (presentRS)
    {
        presentRS->Release();
        presentRS = nullptr;
    }
    if (depthOnlyBasicRS)
    {
        depthOnlyBasicRS->Release();
        depthOnlyBasicRS = nullptr;
    }
    if (depthOnlySkinnedRS)
    {
        depthOnlySkinnedRS->Release();
        depthOnlySkinnedRS = nullptr;
    }
    if (basicRS)
    {
        basicRS->Release();
        basicRS = nullptr;
    }
    if (skinnedRS)
    {
        skinnedRS->Release();
        skinnedRS = nullptr;
    }
    if (spriteRS)
    {
        spriteRS->Release();
        spriteRS = nullptr;
    }
    if (skyboxRS)
    {
        skyboxRS->Release();
        skyboxRS = nullptr;
    }
    if (secondPassRS)
    {
        secondPassRS->Release();
        secondPassRS = nullptr;
    }
    if (transparentBasicRS)
    {
        transparentBasicRS->Release();
        transparentBasicRS = nullptr;
    }
    if (transparentSkinnedRS)
    {
        transparentSkinnedRS->Release();
        transparentSkinnedRS = nullptr;
    }
    if (transparentSpriteRS)
    {
        transparentSpriteRS->Release();
        transparentSpriteRS = nullptr;
    }
    if (OITResolveRS)
    {
        OITResolveRS->Release();
        OITResolveRS = nullptr;
    }
}

void Graphics::DeletePipelineStates()
{
    if (deformingVertexPSO)
    {
        deformingVertexPSO->Release();
        deformingVertexPSO = nullptr;
    }
    if (presentPSO)
    {
        presentPSO->Release();
        presentPSO = nullptr;
    }
    if (D32ToRgbaPSO)
    {
        D32ToRgbaPSO->Release();
        D32ToRgbaPSO = nullptr;
    }
    if (drawNormalPSO)
    {
        drawNormalPSO->Release();
        drawNormalPSO = nullptr;
    }
    if (drawSkinnedNormalPSO)
    {
        drawSkinnedNormalPSO->Release();
        drawSkinnedNormalPSO = nullptr;
    }
    if (skyboxPSO)
    {
        skyboxPSO->Release();
        skyboxPSO = nullptr;
    }
    if (secondPassPSO)
    {
        secondPassPSO->Release();
        secondPassPSO = nullptr;
    }
    for (UINT itemType = 0; itemType < RENDER_ITEM_TYPE_COUNT; itemType++)
    {
        for (UINT passType = 0; passType < DRAW_PASS_TYPE_COUNT; passType++)
        {
            for (UINT fillMode = 0; fillMode < FILL_MODE_COUNT; fillMode++)
            {
                if (PSO[itemType][passType][fillMode])
                {
                    PSO[itemType][passType][fillMode]->Release();
                    PSO[itemType][passType][fillMode] = nullptr;
                }
            }
        }
    }
}

void Graphics::DeleteRaytracingStates()
{
    if (rtStateObjectProps)
    {
        rtStateObjectProps->Release();
        rtStateObjectProps = nullptr;
    }
    if (rtStateObject)
    {
        rtStateObject->Release();
        rtStateObject = nullptr;
    }
    if (globalRS)
    {
        globalRS->Release();
        globalRS = nullptr;
    }
    if (localHitRS)
    {
        localHitRS->Release();
        localHitRS = nullptr;
    }
    if (localEmptyRS)
    {
        localEmptyRS->Release();
        localEmptyRS = nullptr;
    }
    if (rayGenLibrary)
    {
        rayGenLibrary->Release();
        rayGenLibrary = nullptr;
    }
    if (hitLibrary)
    {
        hitLibrary->Release();
        hitLibrary = nullptr;
    }
    if (missLibrary)
    {
        missLibrary->Release();
        missLibrary = nullptr;
    }
    if (shadowLibrary)
    {
        shadowLibrary->Release();
        shadowLibrary = nullptr;
    }
}

void Graphics::SerializeAndCreateRootSignature(ID3D12Device5 *pDevice, const D3D12_ROOT_SIGNATURE_DESC *pDesc,
                                               ID3D12RootSignature **ppOutRS, const WCHAR *rootSigName)
{
    HRESULT              hr = S_OK;
    ID3DBlob            *pSignature = nullptr;
    ID3DBlob            *pError = nullptr;
    ID3D12RootSignature *pRootSignature = nullptr;

    hr = D3D12SerializeRootSignature(pDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError);
    if (FAILED(hr))
        __debugbreak();

    if (FAILED(pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(),
                                            IID_PPV_ARGS(&pRootSignature))))
        __debugbreak();

    pRootSignature->SetName(rootSigName);
    (*ppOutRS) = pRootSignature;

    if (pSignature)
    {
        pSignature->Release();
        pSignature = nullptr;
    }
    if (pError)
    {
        pError->Release();
        pError = nullptr;
    }
}

void Graphics::SerializeAndCreateVersionedRootSignature(ID3D12Device5                             *pDevice,
                                                        const D3D12_VERSIONED_ROOT_SIGNATURE_DESC *pDesc,
                                                        D3D_ROOT_SIGNATURE_VERSION                 version,
                                                        ID3D12RootSignature **ppOutRS, const WCHAR *rootSigName)
{
    HRESULT              hr = S_OK;
    ID3DBlob            *pSignature = nullptr;
    ID3DBlob            *pError = nullptr;
    ID3D12RootSignature *pRootSignature = nullptr;

    hr = D3DX12SerializeVersionedRootSignature(pDesc, version, &pSignature, &pError);
    if (FAILED(hr))
    {
        OutputDebugStringA((LPCSTR)pError->GetBufferPointer());
        __debugbreak();
    }

    if (FAILED(pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(),
                                            IID_PPV_ARGS(&pRootSignature))))
        __debugbreak();

    pRootSignature->SetName(rootSigName);
    (*ppOutRS) = pRootSignature;

    if (pSignature)
    {
        pSignature->Release();
        pSignature = nullptr;
    }
    if (pError)
    {
        pError->Release();
        pError = nullptr;
    }
}
