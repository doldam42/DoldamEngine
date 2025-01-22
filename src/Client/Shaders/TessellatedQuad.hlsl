#include "Common.hlsli"

cbuffer MeshConstantBuffer : register(b1) { MeshConstant meshCB[256]; };

float3 BilinearInterpolation(float3 quad[4], float2 uv)
{
    float3 v1 = lerp(quad[0], quad[1], uv.x);
    float3 v2 = lerp(quad[2], quad[3], uv.x);
    float3 p = lerp(v1, v2, uv.y);

    return p;
}

float2 BilinearInterpolation(float2 quad[4], float2 uv)
{
    float2 v1 = lerp(quad[0], quad[1], uv.x);
    float2 v2 = lerp(quad[2], quad[3], uv.x);
    float2 p = lerp(v1, v2, uv.y);

    return p;
}

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
};

struct PatchConstOutput
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;

    float3 normalWorld[4] : NORMAL;
    float2 texcoord[4] : TEXCOORD;
    float3 tangentWorld[4] : TANGENT;
};

PatchConstOutput MyPatchConstantFunc(InputPatch<HSInput, 4> patch, uint patchID : SV_PrimitiveID)
{
    PatchConstOutput pt;

    const float tessfactor = 8;
    
    pt.edges[0] = tessfactor;
    pt.edges[1] = tessfactor;
    pt.edges[2] = tessfactor;
    pt.edges[3] = tessfactor;
    pt.inside[0] = tessfactor;
    pt.inside[1] = tessfactor;

    pt.normalWorld[0] = patch[0].normalWorld;
    pt.normalWorld[1] = patch[1].normalWorld;
    pt.normalWorld[2] = patch[2].normalWorld;
    pt.normalWorld[3] = patch[3].normalWorld;
    
    pt.texcoord[0] = patch[0].texcoord;
    pt.texcoord[0] = patch[1].texcoord;
    pt.texcoord[0] = patch[2].texcoord;
    pt.texcoord[0] = patch[3].texcoord;

    pt.tangentWorld[0] = patch[0].tangentWorld;
    pt.tangentWorld[1] = patch[1].tangentWorld;
    pt.tangentWorld[2] = patch[2].tangentWorld;
    pt.tangentWorld[3] = patch[3].tangentWorld;

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

    //output.posWorld = input.posModel;
    //output.normalWorld = input.normalModel;
    //output.tangentWorld = input.tangentModel;
    //output.texcoord = input.texcoord;

    return output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("MyPatchConstantFunc")]
[maxtessfactor(64.0f)] 
DSInput HSMain(InputPatch<HSInput, 4> p, uint  i
           : SV_OutputControlPointID, uint patchId
           : SV_PrimitiveID)
{
    DSInput output;

    output.posWorld = p[i].posWorld;

    return output;
}

[domain("quad")]
PSInput DSMain(PatchConstOutput patchConst,
             float2 uv : SV_DomainLocation,
             const OutputPatch<DSInput, 4> quad)
{
    PSInput output;

    float3 posQuad[4] = {quad[0].posWorld, quad[1].posWorld, quad[2].posWorld, quad[3].posWorld};
    float3 posWorld = BilinearInterpolation(posQuad, uv);

    float3 normalQuad[4] = {patchConst.normalWorld[0], patchConst.normalWorld[1], patchConst.normalWorld[2],
                            patchConst.normalWorld[3]};
    float3 normalWorld = BilinearInterpolation(normalQuad, uv);

    float2 texcoordQuad[4] = {patchConst.texcoord[0], patchConst.texcoord[1], patchConst.texcoord[2],
                              patchConst.texcoord[3]};
    float2 texcoord = BilinearInterpolation(texcoordQuad, uv);

    float3 tangentQuad[4] = {patchConst.tangentWorld[0], patchConst.tangentWorld[1], patchConst.tangentWorld[2],
                             patchConst.tangentWorld[3]};
    float3 tangentWorld = BilinearInterpolation(tangentQuad, uv);

    output.posProj = mul(float4(posWorld, 1.0), viewProj);
    output.posWorld = posWorld;
    output.normalWorld = normalWorld;
    output.texcoord = texcoord;
    output.tangentWorld = tangentWorld;

    output.projTexcoord = mul(float4(posWorld, 1.0), projectionViewProj);
    
    return output;
}
