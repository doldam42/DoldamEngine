#include "Common.hlsli"

cbuffer MeshConstantBuffer : register(b1) { MeshConstant meshCB[256]; };

cbuffer cb : register(b5)
{
    uint materialId;
    uint useMaterial;
    uint useHeightMap;
    uint cbDummy[61];
};

Texture2D heightTex : register(t5);

float3 BilinearInterpolationFloat3(float3 quad[4], float2 uv)
{
    // Perform Bilinear Interpolation
    float3 top = lerp(quad[0], quad[1], uv.x);
    float3 bottom = lerp(quad[3], quad[2], uv.x);

    return lerp(top, bottom, uv.y); 
}

float2 BilinearInterpolationFloat2(float2 quad[4], float2 uv)
{
    // Perform Bilinear Interpolation
    float2 top = lerp(quad[0], quad[1], uv.x); 
    float2 bottom = lerp(quad[3], quad[2], uv.x); 

    return lerp(top, bottom, uv.y); 
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
    float3 normalWorld : NORMAL;
    float2 texcoord : TEXCOORD;
    float3 tangentWorld : TANGENT;
};

struct PatchConstOutput
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

PatchConstOutput MyPatchConstantFunc(InputPatch<HSInput, 4> patch, uint patchID : SV_PrimitiveID)
{
    PatchConstOutput pt;

    const float tessfactor = 64.0;
    
    pt.edges[0] = tessfactor;
    pt.edges[1] = tessfactor;
    pt.edges[2] = tessfactor;
    pt.edges[3] = tessfactor;
    pt.inside[0] = tessfactor;
    pt.inside[1] = tessfactor;

    return pt;
}

HSInput VSMain(VSInput input)
{ 
    HSInput output;

    const float4x4 world = meshCB[input.instanceId].world;

    float3 posWorld = mul(float4(input.posModel, 1.0), world).xyz;
    float3 normalWorld = mul(float4(input.normalModel, 0.0f), world).xyz;
    float3 tangentWorld = mul(float4(input.tangentModel, 0.0f), world).xyz;

    // displacement mapping
    if (useHeightMap)
    {
        float height = heightTex.SampleLevel(linearClampSampler, input.texcoord, 0).r;
        height = height * 2.0 - 1.0; // [0, 1] -> [-1, 1]
        posWorld += normalWorld * height;
    }

    output.posWorld = posWorld;
    output.normalWorld = normalize(normalWorld);
    output.texcoord = input.texcoord;
    output.tangentWorld = normalize(tangentWorld);
    
    return output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("MyPatchConstantFunc")]
[maxtessfactor(64.0f)] 
DSInput HSMain(InputPatch<HSInput, 4> p, uint  uCPID : SV_OutputControlPointID)
{
    DSInput output;

    output.posWorld = p[uCPID].posWorld;
    output.normalWorld = p[uCPID].normalWorld;
    output.tangentWorld = p[uCPID].tangentWorld;
    output.texcoord = p[uCPID].texcoord;

    return output;
}

[domain("quad")]
PSInput DSMain(PatchConstOutput patchConst,
             float2 uv : SV_DomainLocation,
             const OutputPatch<DSInput, 4> quad)
{
    PSInput output;

    float3 posQuad[4] = {quad[0].posWorld, quad[1].posWorld, quad[2].posWorld, quad[3].posWorld};
    float3 posWorld = BilinearInterpolationFloat3(posQuad, uv);
    
    float3 normalQuad[4] = { quad[0].normalWorld, quad[1].normalWorld, quad[2].normalWorld, quad[3].normalWorld };
    float3 normalWorld = BilinearInterpolationFloat3(normalQuad, uv);
    
    float2 texcoordQuad[4] = { quad[0].texcoord, quad[1].texcoord, quad[2].texcoord, quad[3].texcoord };
    float2 texcoord = BilinearInterpolationFloat2(texcoordQuad, uv);
    
    float3 tangentQuad[4] = { quad[0].tangentWorld, quad[1].tangentWorld, quad[2].tangentWorld, quad[3].tangentWorld };
    float3 tangentWorld = BilinearInterpolationFloat3(tangentQuad, uv);

    // displacement mapping
    if (useHeightMap)
    {
        float height = heightTex.SampleLevel(linearClampSampler, texcoord, 0).r;
        height = height * 2.0 - 1.0; // [0, 1] -> [-1, 1]
        posWorld += normalWorld * height;
    }

    output.posProj = mul(float4(posWorld, 1.0), viewProj);
    output.posWorld = posWorld;
    output.normalWorld = normalWorld;
    output.texcoord = texcoord;
    output.tangentWorld = tangentWorld;

    output.projTexcoord = mul(float4(posWorld, 1.0), projectionViewProj);
    
    return output;
}
