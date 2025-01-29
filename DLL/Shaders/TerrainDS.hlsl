#include "Common.hlsli"

cbuffer TerrainConstantBuffer : register(b1)
{
    uint  numSlice;
    uint  numStack;
    float scaleX;
    float scaleY;
    float scaleZ;
    float tessFactor;
    uint  tcDummy[58];
}

Texture2D heightTex : register(t5);

float3 BilinearInterpolationFloat3(float3 quad[4], float2 uv)
{
    // Perform Bilinear Interpolation
    float3 top = lerp(quad[2], quad[3], uv.x);
    float3 bottom = lerp(quad[0], quad[1], uv.x);

    return lerp(top, bottom, uv.y);
}

float2 BilinearInterpolationFloat2(float2 quad[4], float2 uv)
{
    // Perform Bilinear Interpolation
    float2 top = lerp(quad[2], quad[3], uv.x);
    float2 bottom = lerp(quad[0], quad[1], uv.x);

    return lerp(top, bottom, uv.y);
}

struct PatchConstOutput
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};
struct DSInput
{
    float3 posWorld : POSITION;
    float2 texcoord : TEXCOORD;
};

[domain("quad")]
PSInput DSMain(PatchConstOutput patchConst, float2 uv : SV_DomainLocation, const OutputPatch<DSInput, 4> quad)
{
    const float3 normalWorld = float3(0.0, 1.0, 0.0);
    const float3 tangentWorld = float3(1.0, 0.0, 0.0); 

    float3 posQuad[4] = {quad[0].posWorld, quad[1].posWorld, quad[2].posWorld, quad[3].posWorld};
    float3 posWorld = BilinearInterpolationFloat3(posQuad, uv);

    float2 texcoordQuad[4] = {quad[0].texcoord, quad[1].texcoord, quad[2].texcoord, quad[3].texcoord};
    float2 texcoord = BilinearInterpolationFloat2(texcoordQuad, uv);

    float height = heightTex.SampleLevel(linearClampSampler, texcoord, 0).r; // displacement mapping
    height = height * 2.0 - 1.0; // UNORM -> SNORM
    posWorld += normalWorld * height * scaleY;

    PSInput output;
    output.posWorld = posWorld;
    output.normalWorld = normalWorld;
    output.texcoord = texcoord;
    output.tangentWorld = tangentWorld;
    output.posProj = mul(float4(posWorld, 1.0), viewProj);
    output.projTexcoord = mul(float4(posWorld, 1.0), projectionViewProj);

    return output;
}