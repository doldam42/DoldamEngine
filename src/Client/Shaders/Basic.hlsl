#include "Common.hlsli"

#include "BxDF.hlsli"
#include "HlslUtils.hlsli"

//////////////////////////////////////////////
// Defines
//////////////////////////////////////////////

static const float4x4 bias = float4x4(0.5, 0, 0, 0, 0, 0.5, 0, 0, 0, 0, 0.5, 0, 0.5, 0.5, 0.5, 1);
static const float    defaultAmbientIntensity = 0.04f;
static const float    smallValue = 1e-6f;
//////////////////////////////////////////////
// Structs
//////////////////////////////////////////////

struct VSInput
{
    float3 posModel : POSITION;  // 모델 좌표계의 위치 position
    float3 normalModel : NORMAL; // 모델 좌표계의 normal
    float2 texcoord : TEXCOORD0;
    float3 tangentModel : TANGENT0;
    uint   instanceId : SV_InstanceID;

#ifdef SKINNED
    float4 boneWeights : BLENDWEIGHT;
    uint4  boneIndices : BLENDINDICES;
#endif
};

struct PSInput
{
    float4 posProj : SV_POSITION; // Screen position
    float3 posWorld : POSITION;   // World position (조명 계산에 사용)
    float3 normalWorld : NORMAL;
    float2 texcoord : TEXCOORD0;
    float4 projTexcoord : TEXCOORD1;
    float3 tangentWorld : TANGENT;
};

struct MeshConstant
{
    float4x4 world;
    float4x4 worldIT;
    float4   mcDummy[8];
};

struct Material
{
    float3 albedo;
    float  roughnessFactor;
    float  metallicFactor;
    float3 emissive;

    float opacityFactor;
    float reflectionFactor;

    uint  flags;
    float dummy[5];
};

//////////////////////////////////////////////
// Constant Buffers
//////////////////////////////////////////////

cbuffer MeshConstantBuffer : register(b1) { MeshConstant meshCB[256]; };

#ifdef SKINNED
cbuffer SkinnedConstantBuffer : register(b2)
{
    float4x4 boneTransforms[64]; // 관절 개수에 따라 조절
}
#endif

//////////////////////////////////////////////
// Resources
//////////////////////////////////////////////

StructuredBuffer<Material> g_materials : register(t20);

// 메쉬 재질 텍스춰들 t0 부터 시작
Texture2D albedoTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D aoTex : register(t2);
Texture2D metallicRoughnessTex : register(t3);
Texture2D emissiveTex : register(t4);

cbuffer cb : register(b5)
{
    uint materialId;
    uint useMaterial;
    uint useHeightMap;
    uint cbDummy[61];
};

//////////////////////////////////////////////
// Helper Functions
//////////////////////////////////////////////

float3 NormalMap(in float3 normal, in float3 tangent, in float2 texcoord)
{
    float3 texSample = normalTex.Sample(linearWrapSampler, texcoord).rgb;
    float3 bumpNormal = normalize(texSample * 2.0f - 1.0f);
    return BumpMapNormalToWorldSpaceNormal(bumpNormal, normal, tangent);
}

bool IsInShadow(float3 posWorld, float4x4 lightViewProj)
{
    if (light.type & LIGHT_SHADOW)
    {
        const float nearZ = 0.01;

        // 1. Project posWorld to light screen
        float4 lightScreen = mul(float4(posWorld, 1.0), lightViewProj);
        lightScreen.xyz /= lightScreen.w;

        // 2. 카메라에서 볼 때의 텍스쳐 좌표 계산
        float2 lightTexcoord = float2(lightScreen.x, -lightScreen.y);
        lightTexcoord += 1.0;
        lightTexcoord *= 0.5;

        // 3. 셰도우 맵에서 값 가져오기
        float depth = shadowMap.Sample(shadowPointSampler, lightTexcoord).r;

        if (depth + 1e-3 < lightScreen.z)
            return true;
    }
    return false;
}

float3 DiffuseIBL(float3 albedo, float3 normalWorld, float3 pixelToEye, float metallic)
{
    float3 F0 = lerp(Fdielectric, albedo, metallic);
    float3 F = SchlickFresnel(F0, max(0.0, dot(normalWorld, pixelToEye)));
    float3 kd = lerp(1.0 - F, 0.0, metallic);
    float3 irradiance = irradianceIBLTex.SampleLevel(linearWrapSampler, normalWorld, 0).rgb;

    return kd * albedo * irradiance;
}

float3 SpecularIBL(float3 albedo, float3 normalWorld, float3 pixelToEye, float metallic, float roughness)
{
    float2 specularBRDF =
        brdfTex.SampleLevel(linearClampSampler, float2(dot(normalWorld, pixelToEye), 1.0 - roughness), 0.0f).rg;
    float3 specularIrradiance =
        specularIBLTex.SampleLevel(linearWrapSampler, reflect(-pixelToEye, normalWorld), 2 + roughness * 5.0f).rgb;
    const float3 Fdielectric = 0.04; // 비금속(Dielectric) 재질의 F0
    float3       F0 = lerp(Fdielectric, albedo, metallic);

    return (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;
}

float3 AmbientLightingByIBL(float3 normalW, float3 pixelToEye, Material material, float ao)
{
    const float3 albedo = material.albedo;
    const float  metallic = material.metallicFactor;
    const float  roughness = material.roughnessFactor;
    float3 diffuseIBL = DiffuseIBL(albedo, normalW, pixelToEye, metallic);
    float3 specularIBL = SpecularIBL(albedo, normalW, pixelToEye, metallic, roughness);

    return (diffuseIBL + specularIBL) * ao;
}

float3 Shade(in float3 N, in float3 V, in float3 posWorld, in Material material)
{
    const float3 Fdielectric = 0.04;                   // 비금속(Dielectric) 재질의 F0
    const uint   materialType = MATERIAL_TYPE_DEFAULT; // 지금은 Default Material로 고정
    
    float3 L = 0;

    // Shadowing
    const float3   lightPos = lights[0].position;
    const float3   radiance = lights[0].radiance;
    const float3   lightDir = lights[0].direction;
    const float4x4 lightViewProj = lights[0].viewProj;

    const float Kr = material.reflectionFactor;
    const float Kt = 1 - material.opacityFactor;
    const float metallic = material.metallicFactor;
    const float roughness = material.roughnessFactor;

    const float3 Kd = material.albedo;
    const float3 Ks = lerp(Fdielectric, Kd, metallic);

    const float3 emissive = material.emissive;

    // Direct illumination
    if (!BxDF::IsBlack(Kd) || !BxDF::IsBlack(Ks))
    {
        // float3 wi = normalize(lightPos - hitPosition);
        float3 wi = -lightDir;

        // Raytraced shadows.
        bool isInShadow = IsInShadow(posWorld, ligth.viewProj);

        L += BxDF::DirectLighting::Shade(materialType, Kd, Ks, radiance, isInShadow, roughness, N, V, wi);
    }

    L += defaultAmbientIntensity * Kd;

    //
    // TODO: Specular Indirect Illumination
    //

    L += emissive;

    return L;
}

// -------------------------------------
PSInput VSMain(VSInput input)
{
    PSInput output;

    MeshConstant meshConst = meshCB[input.instanceId];

#ifdef SKINNED

    float weights[4];
    weights[0] = input.boneWeights.x;
    weights[1] = input.boneWeights.y;
    weights[2] = input.boneWeights.z;
    weights[3] = input.boneWeights.w;

    uint indices[4]; // 힌트: 꼭 사용!
    indices[0] = input.boneIndices.x;
    indices[1] = input.boneIndices.y;
    indices[2] = input.boneIndices.z;
    indices[3] = input.boneIndices.w;

    float3 posModel = float3(0.0f, 0.0f, 0.0f);
    float3 normalModel = float3(0.0f, 0.0f, 0.0f);
    float3 tangentModel = float3(0.0f, 0.0f, 0.0f);

    // Uniform Scaling 가정
    // (float3x3)boneTransforms 캐스팅으로 Translation 제외
    for (int i = 0; i < 4; ++i)
    {
        posModel += weights[i] * mul(float4(input.posModel, 1.0f), boneTransforms[indices[i]]).xyz;
        normalModel += weights[i] * mul(input.normalModel, (float3x3)boneTransforms[indices[i]]);
        tangentModel += weights[i] * mul(input.tangentModel, (float3x3)boneTransforms[indices[i]]);
    }

    input.posModel = posModel;
    input.normalModel = normalModel;
    input.tangentModel = tangentModel;
#endif

    float4 posWorld = mul(float4(input.posModel, 1.0), meshConst.world);

    output.posWorld = posWorld.xyz;

    float4 posProj = mul(posWorld, viewProj);
    output.posProj = posProj;

    output.texcoord = input.texcoord;

    output.projTexcoord = mul(posWorld, projectionViewProj);

    output.normalWorld = mul(float4(input.normalModel, 0.0f), meshConst.world).xyz;
    output.normalWorld = normalize(output.normalWorld);

    output.tangentWorld = mul(float4(input.tangentModel, 0.0f), meshConst.world).xyz;
    output.tangentWorld = normalize(output.tangentWorld);

    return output;
}

// -------------------------------------
float4 PSMain(PSInput input) : SV_TARGET
{
    const uint materialType = MATERIAL_TYPE_DEFAULT; // 지금은 Default Material로 고정

    Material material = g_materials[materialId];

    float3 pixelToEye = normalize(eyeWorld - input.posWorld);

    float3 normalWorld = (material.flags & MATERIAL_USE_NORMAL_MAP)
                             ? NormalMap(input.normalWorld, input.tangentWorld, input.texcoord)
                             : input.normalWorld;

    material.albedo = (material.flags & MATERIAL_USE_ALBEDO_MAP) ? albedoTex.Sample(linearWrapSampler, texcoord).xyz
                                                                 : material.albedo;

    material.roughnessFactor = (material.flags & MATERIAL_USE_ROUGHNESS_MAP)
                                   ? metallicRoughnessTex.Sample(linearWrapSampler, texcoord, lodLevel).g
                                   : material.roughnessFactor;

    material.metallicFactor = (material.flags & MATERIAL_USE_METALLIC_MAP)
                                  ? metallicRoughnessTex.Sample(linearWrapSampler, texcoord, lodLevel).b
                                  : material.metallicFactor;

    material.emissive = (material.flags & MATERIAL_USE_EMISSIVE_MAP)
                            ? emissiveTex.SampleLevel(linearWrapSampler, texcoord, lodLevel).xyz
                            : material.emissive;

    if (useTextureProjection)
    {
        float2 projTexcoord;
        projTexcoord.x = input.projTexcoord.x / input.projTexcoord.w * 0.5 + 0.5;
        projTexcoord.y = -input.projTexcoord.y / input.projTexcoord.w * 0.5 + 0.5;
        if ((saturate(projTexcoord.x) == projTexcoord.x) && (saturate(projTexcoord.y) == projTexcoord.y))
        {
            material.albedo = projectionTex.Sample(linearWrapSampler, projTexcoord).xyz;
        }
    }

    float ao = (material.flags & MATERIAL_USE_AO_MAP) ? aoTex.Sample(linearWrapSampler, input.texcoord).r : 1.0;

    float3 ambientLighting = AmbientLightingByIBL(normalWorld, pixelToEye, material, ao) * 0.2f;
    
    float3 
    float4 output;
    output = float4(ambientLighting + directLighting + emission, opacity);
    output = clamp(output, 0.0, 1000.0);
    return output;
}