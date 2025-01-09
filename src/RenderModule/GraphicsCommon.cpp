#include "pch.h"

#include <DirectXTK/CommonStates.h>
#include <d3dcompiler.h>

#include "D3D12Renderer.h"

#include "GraphicsCommon.h"

#define SizeOfInUint32(obj) ((sizeof(obj) - 1) / sizeof(UINT32) + 1)

namespace Graphics
{

// Shaders
// Vertex Shader
ID3DBlob *vertexShaders[RENDER_ITEM_TYPE_COUNT] = {nullptr};

// Pixel Shader
ID3DBlob *pixelShaders[RENDER_ITEM_TYPE_COUNT] = {nullptr};

// Depth Only Shader
ID3DBlob *depthOnlyVS = nullptr;
ID3DBlob *depthOnlyPS = nullptr;
ID3DBlob *depthOnlySkinningVS = nullptr;

// Draw Normal Shader
ID3DBlob *drawSkinnedNormalVS = nullptr;

ID3DBlob *drawNormalVS = nullptr;
ID3DBlob *drawNormalGS = nullptr;
ID3DBlob *drawNormalPS = nullptr;

// Compute Shader
ID3DBlob *deformingVertexCS = nullptr;

// Present Shader
ID3DBlob *presentVS = nullptr;
ID3DBlob *presentPS = nullptr;

// D32 to R8G8B8A8_UNORM Shader
ID3DBlob *D32ToRgbaPS = nullptr;

// Blend States
D3D12_BLEND_DESC blendStates[DRAW_PASS_TYPE_COUNT] = {};

// DepthStencil States
D3D12_DEPTH_STENCIL_DESC depthStencilStates[DRAW_PASS_TYPE_COUNT] = {};

// Rasterizer States
D3D12_RASTERIZER_DESC rasterizerStates[DRAW_PASS_TYPE_COUNT][FILL_MODE_COUNT] = {};

// Sampler States
D3D12_STATIC_SAMPLER_DESC samplerStates[SAMPLER_TYPE_COUNT] = {};

// RootSignature
ID3D12RootSignature *emptyRS = nullptr;
ID3D12RootSignature *deformingVertexRS = nullptr;
ID3D12RootSignature *presentRS = nullptr;

ID3D12RootSignature *depthOnlyBasicRS = nullptr;
ID3D12RootSignature *depthOnlySkinnedRS = nullptr;

ID3D12RootSignature *rootSignatures[RENDER_ITEM_TYPE_COUNT] = {nullptr};

// Pipeline State Objects
ID3D12PipelineState *deformingVertexPSO = nullptr;
ID3D12PipelineState *presentPSO = nullptr;
ID3D12PipelineState *D32ToRgbaPSO = nullptr;
ID3D12PipelineState *drawNormalPSO = nullptr;
ID3D12PipelineState *drawSkinnedNormalPSO = nullptr;

ID3D12PipelineState *PSO[RENDER_ITEM_TYPE_COUNT][DRAW_PASS_TYPE_COUNT][FILL_MODE_COUNT] = {nullptr};

D3D12_INPUT_ELEMENT_DESC simpleIEs[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

D3D12_INPUT_ELEMENT_DESC basicIEs[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};
D3D12_INPUT_ELEMENT_DESC skinnedIEs[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 60, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
};

D3D12_INPUT_LAYOUT_DESC inputLayouts[RENDER_ITEM_TYPE_COUNT] = {nullptr};

// #DXR
const wchar_t *missShaderNames[2] = {L"Miss", L"ShadowMiss"};
const wchar_t *hitShaderNames[2] = {L"ClosestHit", L"ShadowClosestHit"};
const wchar_t *hitGroupNames[2] = {L"HitGroup", L"ShadowHitGroup"};

IDxcBlob *rayGenLibrary = nullptr;
IDxcBlob *missLibrary = nullptr;
IDxcBlob *hitLibrary = nullptr;
IDxcBlob *shadowLibrary = nullptr;

ID3D12RootSignature *globalRS = nullptr;
ID3D12RootSignature *localHitRS = nullptr;
ID3D12RootSignature *localEmptyRS = nullptr;

ID3D12StateObject           *rtStateObject = nullptr;
ID3D12StateObjectProperties *rtStateObjectProps = nullptr;
} // namespace Graphics

void Graphics::InitCommonStates(ID3D12Device5 *pD3DDevice)
{
    InitInputLayouts();
    InitBlendStates();
    InitRasterizerStates();
    InitDepthStencilStates();
    InitSamplers();

    InitShaders(pD3DDevice);
    InitRootSignature(pD3DDevice);
    InitPipelineStates(pD3DDevice);

#ifdef USE_RAYTRACING
    InitRaytracingStates(pD3DDevice);
#endif
}

void Graphics::InitInputLayouts()
{
    inputLayouts[RENDER_ITEM_TYPE_CHAR_OBJ] = {skinnedIEs, _countof(skinnedIEs)};

    inputLayouts[RENDER_ITEM_TYPE_MESH_OBJ] = {basicIEs, _countof(basicIEs)};
    inputLayouts[RENDER_ITEM_TYPE_SPRITE] = inputLayouts[RENDER_ITEM_TYPE_SKYBOX] = {simpleIEs, _countof(simpleIEs)};
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
                            compileFlags, 0, &vertexShaders[RENDER_ITEM_TYPE_MESH_OBJ], nullptr);
    if (FAILED(hr))
        __debugbreak();
    hr = D3DCompileFromFile(L"./Shaders/BasicPS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1",
                            compileFlags, 0, &pixelShaders[RENDER_ITEM_TYPE_MESH_OBJ], nullptr);
    if (FAILED(hr))
        __debugbreak();
    hr = D3DCompileFromFile(L"./Shaders/BasicVS.hlsl", skinnedVSMacros, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
                            "vs_5_1", compileFlags, 0, &vertexShaders[RENDER_ITEM_TYPE_CHAR_OBJ], nullptr);
    if (FAILED(hr))
        __debugbreak();

    pixelShaders[RENDER_ITEM_TYPE_CHAR_OBJ] = pixelShaders[RENDER_ITEM_TYPE_MESH_OBJ];

    // SkyBox
    hr = D3DCompileFromFile(L"./Shaders/SkyboxVS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1",
                            compileFlags, 0, &vertexShaders[RENDER_ITEM_TYPE_SKYBOX], nullptr);
    if (FAILED(hr))
        __debugbreak();
    hr = D3DCompileFromFile(L"./Shaders/SkyboxPS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1",
                            compileFlags, 0, &pixelShaders[RENDER_ITEM_TYPE_SKYBOX], nullptr);

    // Sprite
    hr = D3DCompileFromFile(L"./Shaders/SpriteVS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1",
                            compileFlags, 0, &vertexShaders[RENDER_ITEM_TYPE_SPRITE], nullptr);
    if (FAILED(hr))
        __debugbreak();
    hr = D3DCompileFromFile(L"./Shaders/SpritePS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1",
                            compileFlags, 0, &pixelShaders[RENDER_ITEM_TYPE_SPRITE], nullptr);
    if (FAILED(hr))
        __debugbreak();

    // Depth Only
    hr = D3DCompileFromFile(L"./Shaders/DepthOnlyVS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1",
                            compileFlags, 0, &depthOnlyVS, nullptr);
    if (FAILED(hr))
        __debugbreak();
    hr = D3DCompileFromFile(L"./Shaders/DepthOnlyVS.hlsl", skinnedVSMacros, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
                            "vs_5_1", compileFlags, 0, &depthOnlySkinningVS, nullptr);
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
                            "vs_5_1", compileFlags, 0, &drawSkinnedNormalVS, nullptr);
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
    hr = D3DCompileFromFile(L"./Shaders/D32ToRgbaPS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1",
                            compileFlags, 0, &D32ToRgbaPS, nullptr);
    if (FAILED(hr))
        __debugbreak();
}

void Graphics::InitBlendStates()
{
    using namespace DirectX;

    blendStates[DRAW_PASS_TYPE_DEFAULT] = blendStates[DRAW_PASS_TYPE_SHADOW] = CommonStates::Opaque;
    blendStates[DRAW_PASS_TYPE_NON_OPAQUE] = CommonStates::AlphaBlend;
}

void Graphics::InitRasterizerStates()
{
    using namespace DirectX;

    const CD3DX12_RASTERIZER_DESC desc(D3D12_DEFAULT);
    rasterizerStates[DRAW_PASS_TYPE_DEFAULT][FILL_MODE_SOLID] =
        rasterizerStates[DRAW_PASS_TYPE_NON_OPAQUE][FILL_MODE_SOLID] =
            rasterizerStates[DRAW_PASS_TYPE_SHADOW][FILL_MODE_SOLID] = desc;

    rasterizerStates[DRAW_PASS_TYPE_DEFAULT][FILL_MODE_WIRED] =
        rasterizerStates[DRAW_PASS_TYPE_NON_OPAQUE][FILL_MODE_WIRED] =
            rasterizerStates[DRAW_PASS_TYPE_SHADOW][FILL_MODE_WIRED] = CommonStates::Wireframe;
}

void Graphics::InitDepthStencilStates()
{
    using namespace DirectX;

    depthStencilStates[DRAW_PASS_TYPE_DEFAULT] = depthStencilStates[DRAW_PASS_TYPE_SHADOW] = CommonStates::DepthDefault;
    depthStencilStates[DRAW_PASS_TYPE_NON_OPAQUE] = CommonStates::DepthRead;
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

    ID3D12RootSignature *basicRS = nullptr;
    ID3D12RootSignature *skinnedRS = nullptr;
    ID3D12RootSignature *skyboxRS = nullptr;
    ID3D12RootSignature *spriteRS = nullptr;

    // This is the highest version the sample supports. If CheckFeatureSupport
    // succeeds, the HighestVersion returned will not be greater than this.
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

    hr = pD3DDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData));
    if (FAILED(hr))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    ID3DBlob *pSignature = nullptr;
    ID3DBlob *pError = nullptr;

    // Create an empty root signature.
    {
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError)))
        {
            __debugbreak();
        }

        if (FAILED(pD3DDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(),
                                                   IID_PPV_ARGS(&emptyRS))))
        {
            __debugbreak();
            goto lb_return;
        }

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

    // Ranges Per Global
    // | Global Consts(b0) | 
    // | Materials (t5)    |
    // | EnvIBL(t10)       | SpecularIBL(t11) | irradianceIBL(t12) | brdfIBL(t13) | ProjectionTex(t14) |
    // | ShadowMap1(t15)   | ShadowMap2(t16)  | ShadowMap3(t15) |
    CD3DX12_DESCRIPTOR_RANGE1 rangesPerGlobal[4];
    rangesPerGlobal[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);           // b0 : globalConsts
    rangesPerGlobal[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5);           // t5 : materials
    rangesPerGlobal[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 10);          // t10~13 : IBL Textures, t14 : ProjectionTex
    rangesPerGlobal[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, MAX_LIGHTS, 15); // t15~t15+MAX_LIGHT : shadow maps
    // Init Basic Root Signature
    {
        CD3DX12_DESCRIPTOR_RANGE1 rangesPerObj[1];
        CD3DX12_DESCRIPTOR_RANGE1 rangesPerMesh[2];

        CD3DX12_ROOT_PARAMETER1 rootParameters[3];

        // Root Paramaters
        // | Global Consts |  Materials  | IBL Textures(t10) | ShadowMaps(t15) |
        // |   TR Matrix   |
        // |   Geometry    |  albedoTex(t0) | normalTex(t1) | aoTex(t2) | metallicRoughnesesTex(t3) | emissiveTex(t4) |
        rangesPerObj[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1,
                             1); // b1 : TR Matrix
        rangesPerMesh[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1,
                              5); // b5 : Geometry
        rangesPerMesh[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5,
                              0); // textures
        rootParameters[0].InitAsDescriptorTable(_countof(rangesPerGlobal), &rangesPerGlobal[0],
                                                D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[1].InitAsDescriptorTable(_countof(rangesPerObj), &rangesPerObj[0], D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[2].InitAsDescriptorTable(_countof(rangesPerMesh), &rangesPerMesh[0],
                                                D3D12_SHADER_VISIBILITY_ALL);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, SAMPLER_TYPE_COUNT, samplerStates,
                                   D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        hr =
            D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &pSignature, &pError);
        if (FAILED(hr))
        {
            __debugbreak();
            goto lb_return;
        }

        if (FAILED(pD3DDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(),
                                                   IID_PPV_ARGS(&basicRS))))
        {
            __debugbreak();
            goto lb_return;
        }

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

    // Init Skinned Root Signature
    {
        CD3DX12_DESCRIPTOR_RANGE1 rangesPerObj[1];
        CD3DX12_DESCRIPTOR_RANGE1 rangesPerMesh[2];
        CD3DX12_ROOT_PARAMETER1   rootParameters[3];

        // Root Paramaters
        // | Global Consts |  Materials  |
        // |   TR Matrix   | Bone Matrix |
        // |   Geometry    |   Textures   |
        rangesPerObj[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2,
                             1); // b1 : TR Matrix, b2 : Bone Matrix
        rangesPerMesh[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1,
                              5); // b5 : Geometry
        rangesPerMesh[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5,
                              0); // t0 : Texture
        rootParameters[0].InitAsDescriptorTable(_countof(rangesPerGlobal), &rangesPerGlobal[0],
                                                D3D12_SHADER_VISIBILITY_ALL); // t5 : Materials
        rootParameters[1].InitAsDescriptorTable(_countof(rangesPerObj), &rangesPerObj[0], D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[2].InitAsDescriptorTable(_countof(rangesPerMesh), &rangesPerMesh[0],
                                                D3D12_SHADER_VISIBILITY_ALL);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, SAMPLER_TYPE_COUNT, samplerStates,
                                   D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        hr =
            D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &pSignature, &pError);
        if (FAILED(hr))
        {
            __debugbreak();
            goto lb_return;
        }

        if (FAILED(pD3DDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(),
                                                   IID_PPV_ARGS(&skinnedRS))))
        {
            __debugbreak();
            goto lb_return;
        }

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

    // Init Skybox Root Signature
    {
        CD3DX12_DESCRIPTOR_RANGE1 rangesPerEnv[1];
        CD3DX12_ROOT_PARAMETER1   rootParameters[2];
        rangesPerEnv[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 10); // | Env | specular | irradiance | brdf |
        rootParameters[0].InitAsConstantBufferView(0);
        rootParameters[1].InitAsDescriptorTable(_countof(rangesPerEnv), &rangesPerEnv[0], D3D12_SHADER_VISIBILITY_ALL);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, SAMPLER_TYPE_COUNT, samplerStates,
                                   D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        hr =
            D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &pSignature, &pError);
        if (FAILED(hr))
        {
            __debugbreak();
            goto lb_return;
        }

        if (FAILED(pD3DDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(),
                                                   IID_PPV_ARGS(&skyboxRS))))
        {
            __debugbreak();
            goto lb_return;
        }

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

    // Init Sprite Root Signature
    {
        CD3DX12_DESCRIPTOR_RANGE ranges[2] = {};
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // b0 : Constant Buffer View
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0 : Shader Resource View(Tex)

        CD3DX12_ROOT_PARAMETER rootParameters[1] = {};
        rootParameters[0].InitAsDescriptorTable(_countof(ranges), ranges, D3D12_SHADER_VISIBILITY_ALL);

        // Allow input layout and deny uneccessary access to certain pipeline stages.
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                                                        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                                                        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                                                        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
                                                        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

        // Create an empty root signature.
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        // rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        rootSignatureDesc.Init(_countof(rootParameters), rootParameters, SAMPLER_TYPE_COUNT, samplerStates,
                               D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError)))
        {
            __debugbreak();
            goto lb_return;
        }

        if (FAILED(pD3DDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(),
                                                   IID_PPV_ARGS(&spriteRS))))
        {
            __debugbreak();
            goto lb_return;
        }

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

    // Init Skinning Root Signature
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
        // | Global Consts |
        // |   TR Matrix   |
        CD3DX12_DESCRIPTOR_RANGE rangePerGlobal[1] = {};
        rangePerGlobal[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // b0

        CD3DX12_DESCRIPTOR_RANGE rangePerMesh[1] = {};
        rangePerMesh[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1); // b1

        CD3DX12_ROOT_PARAMETER rootParameters[2] = {};
        rootParameters[0].InitAsDescriptorTable(_countof(rangePerGlobal), rangePerGlobal, D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[1].InitAsDescriptorTable(_countof(rangePerMesh), rangePerMesh, D3D12_SHADER_VISIBILITY_ALL);

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters, SAMPLER_TYPE_COUNT,
                                                      samplerStates,
                                                      D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        SerializeAndCreateRootSignature(pD3DDevice, &rootSignatureDesc, &depthOnlyBasicRS, L"DepthOnly Basic RS");
    }
    // Init DepthOnly Skinned Root Signature
    {
        // Root Paramaters
        // | Global Consts |
        // |   TR Matrix   | Bone Matrix |
        CD3DX12_DESCRIPTOR_RANGE rangePerGlobal[1] = {};
        rangePerGlobal[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // b0

        CD3DX12_DESCRIPTOR_RANGE rangePerMesh[1] = {};
        rangePerMesh[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 1); // b1~b2

        CD3DX12_ROOT_PARAMETER rootParameters[2] = {};
        rootParameters[0].InitAsDescriptorTable(_countof(rangePerGlobal), rangePerGlobal, D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[1].InitAsDescriptorTable(_countof(rangePerMesh), rangePerMesh, D3D12_SHADER_VISIBILITY_ALL);

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters, SAMPLER_TYPE_COUNT,
                                                      samplerStates,
                                                      D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        SerializeAndCreateRootSignature(pD3DDevice, &rootSignatureDesc, &depthOnlySkinnedRS, L"depthOnly Skinned RS");
    }

    rootSignatures[RENDER_ITEM_TYPE_MESH_OBJ] = basicRS;
    rootSignatures[RENDER_ITEM_TYPE_CHAR_OBJ] = skinnedRS;
    rootSignatures[RENDER_ITEM_TYPE_SPRITE] = spriteRS;
    rootSignatures[RENDER_ITEM_TYPE_SKYBOX] = skyboxRS;

lb_return:
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

    for (UINT itemType = 0; itemType < RENDER_ITEM_TYPE_COUNT; itemType++)
    {
        psoDesc.InputLayout = inputLayouts[itemType];
        psoDesc.pRootSignature = rootSignatures[itemType];
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShaders[itemType]->GetBufferPointer(),
                                             vertexShaders[itemType]->GetBufferSize());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShaders[itemType]->GetBufferPointer(),
                                             pixelShaders[itemType]->GetBufferSize());
        for (UINT passType = 0; passType < DRAW_PASS_TYPE_SHADOW; passType++)
        {
            psoDesc.BlendState = blendStates[passType];
            psoDesc.DepthStencilState = depthStencilStates[passType];
            for (UINT fillMode = 0; fillMode < FILL_MODE_COUNT; fillMode++)
            {
                psoDesc.RasterizerState = rasterizerStates[passType][fillMode];

                if (FAILED(pD3DDevice->CreateGraphicsPipelineState(&psoDesc,
                                                                   IID_PPV_ARGS(&PSO[itemType][passType][fillMode]))))
                {
                    __debugbreak();
                }
            }
        }
    }

    // shadowMap
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.InputLayout = inputLayouts[RENDER_ITEM_TYPE_MESH_OBJ];
    psoDesc.pRootSignature = depthOnlyBasicRS;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(depthOnlyVS->GetBufferPointer(), depthOnlyVS->GetBufferSize());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(depthOnlyPS->GetBufferPointer(), depthOnlyPS->GetBufferSize());

    psoDesc.BlendState = blendStates[DRAW_PASS_TYPE_SHADOW];
    psoDesc.DepthStencilState = depthStencilStates[DRAW_PASS_TYPE_SHADOW];

    for (UINT fillMode = 0; fillMode < FILL_MODE_COUNT; fillMode++)
    {
        psoDesc.RasterizerState = rasterizerStates[DRAW_PASS_TYPE_SHADOW][fillMode];
        if (FAILED(pD3DDevice->CreateGraphicsPipelineState(
                &psoDesc, IID_PPV_ARGS(&PSO[RENDER_ITEM_TYPE_MESH_OBJ][DRAW_PASS_TYPE_SHADOW][fillMode]))))
        {
            __debugbreak();
        }
    }

    psoDesc.InputLayout = inputLayouts[RENDER_ITEM_TYPE_CHAR_OBJ];
    psoDesc.pRootSignature = depthOnlySkinnedRS;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(depthOnlySkinningVS->GetBufferPointer(), depthOnlySkinningVS->GetBufferSize());

    for (UINT fillMode = 0; fillMode < FILL_MODE_COUNT; fillMode++)
    {
        psoDesc.RasterizerState = rasterizerStates[DRAW_PASS_TYPE_SHADOW][fillMode];
        if (FAILED(pD3DDevice->CreateGraphicsPipelineState(
                &psoDesc, IID_PPV_ARGS(&PSO[RENDER_ITEM_TYPE_CHAR_OBJ][DRAW_PASS_TYPE_SHADOW][fillMode]))))
        {
            __debugbreak();
        }
    }

    // Draw Normal PSO
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    psoDesc.InputLayout = inputLayouts[RENDER_ITEM_TYPE_MESH_OBJ];
    psoDesc.pRootSignature = depthOnlyBasicRS;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(drawNormalVS->GetBufferPointer(), drawNormalVS->GetBufferSize());
    psoDesc.GS = CD3DX12_SHADER_BYTECODE(drawNormalGS->GetBufferPointer(), drawNormalGS->GetBufferSize());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(drawNormalPS->GetBufferPointer(), drawNormalPS->GetBufferSize());
    if (FAILED(pD3DDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&drawNormalPSO))))
    {
        __debugbreak();
    }

    psoDesc.InputLayout = inputLayouts[RENDER_ITEM_TYPE_CHAR_OBJ];
    psoDesc.pRootSignature = depthOnlySkinnedRS;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(drawSkinnedNormalVS->GetBufferPointer(), drawSkinnedNormalVS->GetBufferSize());
    if (FAILED(pD3DDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&drawSkinnedNormalPSO))))
    {
        __debugbreak();
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
    // present PSO
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {simpleIEs, _countof(simpleIEs)};
        psoDesc.pRootSignature = presentRS;
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(presentVS->GetBufferPointer(), presentVS->GetBufferSize());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(presentPS->GetBufferPointer(), presentPS->GetBufferSize());
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
        // D32 to RGBA PSO

        psoDesc.PS = CD3DX12_SHADER_BYTECODE(D32ToRgbaPS->GetBufferPointer(), D32ToRgbaPS->GetBufferSize());
        if (FAILED(pD3DDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&D32ToRgbaPSO))))
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
    BOOL disableDebug = FALSE;
#ifdef _DEBUG
    disableDebug = TRUE;
#endif // _DEBUG

    rayGenLibrary = CompileShaderLibrary(L"./Shaders/DXR/RayGen.hlsl", disableDebug);
    auto lib = raytracingPipeline->CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    lib->SetDXILLibrary(&CD3DX12_SHADER_BYTECODE(rayGenLibrary->GetBufferPointer(), rayGenLibrary->GetBufferSize()));

    missLibrary = CompileShaderLibrary(L"./Shaders/DXR/Miss.hlsl", disableDebug);
    lib = raytracingPipeline->CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    lib->SetDXILLibrary(&CD3DX12_SHADER_BYTECODE(missLibrary->GetBufferPointer(), missLibrary->GetBufferSize()));

    hitLibrary = CompileShaderLibrary(L"./Shaders/DXR/Hit.hlsl", disableDebug);
    lib = raytracingPipeline->CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    lib->SetDXILLibrary(&CD3DX12_SHADER_BYTECODE(hitLibrary->GetBufferPointer(), hitLibrary->GetBufferSize()));

    shadowLibrary = CompileShaderLibrary(L"./Shaders/DXR/ShadowRay.hlsl", disableDebug);
    lib = raytracingPipeline->CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    lib->SetDXILLibrary(&CD3DX12_SHADER_BYTECODE(shadowLibrary->GetBufferPointer(), shadowLibrary->GetBufferSize()));
}

void Graphics::InitRaytracingRootSignatures(ID3D12Device5 *pD3DDevice)
{
    // Init Global Root Signature
    // |   OUTPUT_VIEW(u0)   | ACCELERATION_STRUCTURE(t0) |
    // | GlobalConstants(b0) |       Materials(t5)        |
    {
        CD3DX12_DESCRIPTOR_RANGE rangesPerGlobal1[2];
        rangesPerGlobal1[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); // u0
        rangesPerGlobal1[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0
        // rangesPerGlobal[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

        CD3DX12_DESCRIPTOR_RANGE rangesPerGlobal2[2];
        rangesPerGlobal2[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
        rangesPerGlobal2[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5);

        CD3DX12_ROOT_PARAMETER rootParameters[2];
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
    // |  VERTICES(t0, space1)  | INDICES(t1, space1) | DIFFUSE_TEX(t2, space1) | 
    {
        CD3DX12_DESCRIPTOR_RANGE ranges[LOCAL_ROOT_PARAM_INDEX_COUNT];
        ranges[LOCAL_ROOT_PARAM_INDEX_VERTICES].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);
        ranges[LOCAL_ROOT_PARAM_INDEX_INDICES].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 1);
        ranges[LOCAL_ROOT_PARAM_INDEX_DIFFUSE_TEX].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 1);

        CD3DX12_ROOT_PARAMETER rootParameters[LOCAL_ROOT_PARAM_INDEX_COUNT];
        rootParameters[LOCAL_ROOT_PARAM_INDEX_CB].InitAsConstants(SizeOfInUint32(RaytracingFaceGroupCB), 0, 1);
        rootParameters[LOCAL_ROOT_PARAM_INDEX_VERTICES].InitAsDescriptorTable(1,
                                                                              &ranges[LOCAL_ROOT_PARAM_INDEX_VERTICES]);
        rootParameters[LOCAL_ROOT_PARAM_INDEX_INDICES].InitAsDescriptorTable(1,
                                                                             &ranges[LOCAL_ROOT_PARAM_INDEX_INDICES]);
        rootParameters[LOCAL_ROOT_PARAM_INDEX_DIFFUSE_TEX].InitAsDescriptorTable(
            1, &ranges[LOCAL_ROOT_PARAM_INDEX_DIFFUSE_TEX]);

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
    switch (passType)
    {
    case DRAW_PASS_TYPE_DEFAULT:
    case DRAW_PASS_TYPE_NON_OPAQUE:
        return rootSignatures[itemType];
    case DRAW_PASS_TYPE_SHADOW: {
        if (itemType == RENDER_ITEM_TYPE_MESH_OBJ)
            return depthOnlyBasicRS;
        if (itemType == RENDER_ITEM_TYPE_CHAR_OBJ)
            return depthOnlySkinnedRS;
#ifdef _DEBUG
        __debugbreak();
#endif //  _DEBUG
        break;
    }
    default:
#ifdef _DEBUG
        __debugbreak();
#endif //  _DEBUG
        break;
    }
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
    for (UINT i = 0; i < RENDER_ITEM_TYPE_COUNT; i++)
    {
        if (vertexShaders[i])
        {
            vertexShaders[i]->Release();
            vertexShaders[i] = nullptr;
        }
        if (pixelShaders[i])
        {
            pixelShaders[i]->Release();
            pixelShaders[i] = nullptr;
        }
    }

    if (depthOnlyVS)
    {
        depthOnlyVS->Release();
        depthOnlyVS = nullptr;
    }
    if (depthOnlySkinningVS)
    {
        depthOnlySkinningVS->Release();
        depthOnlySkinningVS = nullptr;
    }
    if (depthOnlyPS)
    {
        depthOnlyPS->Release();
        depthOnlyPS = nullptr;
    }
    if (deformingVertexCS)
    {
        deformingVertexCS->Release();
        deformingVertexCS = nullptr;
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
    if (D32ToRgbaPS)
    {
        D32ToRgbaPS->Release();
        D32ToRgbaPS = nullptr;
    }
    if (drawNormalVS)
    {
        drawNormalVS->Release();
        drawNormalVS = nullptr;
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

    for (UINT i = 0; i < RENDER_ITEM_TYPE_COUNT; i++)
    {
        if (rootSignatures[i])
        {
            rootSignatures[i]->Release();
            rootSignatures[i] = nullptr;
        }
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
