#include "Common.hlsli"

cbuffer MeshConstantBuffer : register(b1) { MeshConstant meshCB[256]; };

Texture2D heightTex : register(t5);

struct HSInput
{
    float3 posWorld : POSITION;
    float3 normalWorld : NORMAL;
    float2 texcoord : TEXCOORD;
    float3 tangentWorld : TANGENT;
};
struct DSInput
{
    float3 posWorld : POSITION;
    float3 normalWorld : NORMAL;
    float2 texcoord : TEXCOORD;
    float3 tangentWorld : TANGENT;
};

struct PatchConstOutput
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

PatchConstOutput MyPatchConstantFunc(InputPatch<HSInput, 3> patch, uint patchID : SV_PrimitiveID)
{
    PatchConstOutput pt;

    const float tessfactor = 64.0;

    pt.edges[0] = tessfactor;
    pt.edges[1] = tessfactor;
    pt.edges[2] = tessfactor;
    pt.inside = tessfactor;

    return pt;
}

HSInput VSMain(VSInput input)
{
    HSInput output;

    const float4x4 world = meshCB[input.instanceId].world;

    float3 posWorld = mul(float4(input.posModel, 1.0), world).xyz;
    float3 normalWorld = mul(float4(input.normalModel, 0.0f), world).xyz;
    float3 tangentWorld = mul(float4(input.tangentModel, 0.0f), world).xyz;

    output.posWorld = posWorld;
    output.normalWorld = normalize(normalWorld);
    output.texcoord = input.texcoord;
    output.tangentWorld = normalize(tangentWorld);

    return output;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("MyPatchConstantFunc")]
[maxtessfactor(64.0f)] 
DSInput HSMain(InputPatch<HSInput, 3> p, uint uCPID : SV_OutputControlPointID)
{
    DSInput output;

    output.posWorld = p[uCPID].posWorld;
    output.normalWorld = p[uCPID].normalWorld;
    output.tangentWorld = p[uCPID].tangentWorld;
    output.texcoord = p[uCPID].texcoord;

    return output;
}

[domain("tri")] 
PSInput DSMain(PatchConstOutput patchConst, float3 uv : SV_DomainLocation, const OutputPatch<DSInput, 3> tri)
{
    PSInput output;

    // Barycentric Coordinate
    float3 posWorld = uv.x * tri[0].posWorld + uv.y * tri[1].posWorld + uv.z * tri[2].posWorld;
    float3 normalWorld = uv.x * tri[0].normalWorld + uv.y * tri[1].normalWorld + uv.z * tri[2].normalWorld;
    float3 texcoord = uv.x * tri[0].texcoord + uv.y * tri[1].texcoord + uv.z * tri[2].texcoord;
    float3 tangentWorld = uv.x * tri[0].tangentWorld + uv.y * tri[1].tangentWorld + uv.z * tri[2].tangentWorld;

    // displacement mapping
    float height = heightTex.SampleLevel(linearClampSampler, texcoord, 0).r;
    height = height * 2.0 - 1.0;
    posWorld += normalWorld * height;

    output.posWorld = posWorld;
    output.normalWorld = normalWorld;
    output.texcoord = texcoord;
    output.tangentWorld = tangentWorld;
    output.posProj = mul(float4(posWorld, 1.0), viewProj);
    output.projTexcoord = mul(float4(posWorld, 1.0), projectionViewProj);

    return output;
}








                    