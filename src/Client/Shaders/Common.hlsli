#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

#define MAX_LIGHTS 3 // 쉐이더에서도 #define 사용 가능
#define LIGHT_OFF 0x00
#define LIGHT_DIRECTIONAL 0x01
#define LIGHT_POINT 0x02
#define LIGHT_SPOT 0x04
#define LIGHT_SHADOW 0x10

SamplerState linearWrapSampler : register(s0);
SamplerState linearClampSampler : register(s1);
SamplerState pointWrapSampler : register(s2);
SamplerState pointClampSampler : register(s3);
SamplerState shadowPointSampler : register(s4);
SamplerState anisotropicWrapSampler : register(s5);
SamplerState anisotropicClampSampler : register(s6);

TextureCube envIBLTex : register(t10);
TextureCube specularIBLTex : register(t11);
TextureCube irradianceIBLTex : register(t12);
Texture2D brdfTex : register(t13);
Texture2D projectionTex : register(t14);

Texture2D shadowMaps[MAX_LIGHTS] : register(t15);

struct Light
{
    float3 radiance; // strength
    float fallOffStart;
    float3 direction;
    float fallOffEnd;
    float3 position;
    float spotPower;

    uint type;
    float radius; // 반지름

    float haloRadius;
    float haloStrength;

    Matrix viewProj; 
    Matrix invProj;
};

struct MeshConstant
{
    float4x4 world;
    float4x4 worldIT;
    float4 mcDummy[8];
};

struct MaterialConstant
{
    float3 albedo;
    float roughnessFactor;
    float metallicFactor;
    float3 emissive;
    
    float opacityFactor;
    float reflectionFactor;
    
    uint useAlbedoMap;
    uint useNormalMap;
    uint useAOMap;
    uint useMetallicMap;
    uint useRoughnessMap;
    uint useEmissiveMap;
};

StructuredBuffer<MaterialConstant> g_materials : register(t5);

cbuffer GlobalConstants : register(b0)
{
    float4x4 view;
    float4x4 invView;
    float4x4 proj;
    float4x4 invProj;
    float4x4 viewProj;
    float4x4 invViewProj; // Proj -> World
    float4x4 projectionViewProj; // texture projection 
    
    float3 eyeWorld;
    float strengthIBL;
    
    Light lights[MAX_LIGHTS];
    uint useTextureProjection;
    float gcDummy2[59];
};

struct VertexShaderInput
{
    float3 posModel : POSITION; //모델 좌표계의 위치 position
    float3 normalModel : NORMAL; // 모델 좌표계의 normal    
    float2 texcoord : TEXCOORD0;
    float3 tangentModel : TANGENT0;
    uint instanceId : SV_InstanceID;
    
#ifdef SKINNED
    float4 boneWeights : BLENDWEIGHT;
    uint4 boneIndices : BLENDINDICES;
#endif
};

struct PixelShaderInput
{
    float4 posProj : SV_POSITION; // Screen position
    float3 posWorld : POSITION; // World position (조명 계산에 사용)
    float3 normalWorld : NORMAL;
    float2 texcoord : TEXCOORD0;
    float4 projTexcoord : TEXCOORD1;
    float3 tangentWorld : TANGENT0;
};

#endif // __COMMON_HLSLI__
