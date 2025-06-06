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

// Vertex Shader Control Point Input
struct VSCPInput
{
    float height : HEIGHT;
    uint  vId : SV_VertexID;
};
struct HSInput
{
    float3 posWorld : POSITION;
    float2 texcoord : TEXCOORD;
};

HSInput VSMain(VSCPInput input)
{
    HSInput output;

    const uint i = input.vId % (numSlice + 1);
    const uint j = input.vId / (numSlice + 1);

    const float dx = 1.0f / numSlice;
    const float dy = 1.0f / numStack;

    const float x = dx * i;
    const float y = dy * j;

    // [0, 0] -> [-1, 0, 1]
    // [1, 0] -> [ 1, 0, 1]
    // [0, 1] -> [-1, 0,-1]
    // [1, 1] -> [ 1, 0,-1]
    // f(x,y) = (2x - 1, 0, 1.0 - 2y)
    output.texcoord = float2(x, y);
    output.posWorld = float3((2 * x - 1.0) * scaleX * 0.5, 0, (1.0 - 2 * y) * scaleZ * 0.5);

    return output;
}

PSInput VSMainWithoutHS(VSCPInput input)
{
    PSInput output;

    const uint i = input.vId % (numSlice + 1);
    const uint j = input.vId / (numSlice + 1);

    const float dx = 1.0f / numSlice;
    const float dy = 1.0f / numStack;

    const float x = dx * i;
    const float y = dy * j;
    const float height = input.height * 2.0 - 1.0;  // [0, 1] -> [-1, 1]

    const float U = heightTex.SampleLevel(linearClampSampler, float2(x, y - dy), 0).r;
    const float D = heightTex.SampleLevel(linearClampSampler, float2(x, y + dy), 0).r;
    const float R = heightTex.SampleLevel(linearClampSampler, float2(x + dx, y), 0).r;
    const float L = heightTex.SampleLevel(linearClampSampler, float2(x - dx, y), 0).r;

    const float3 T = float3(1.0, (R - L), 0.0);
    const float3 B = float3(0.0, (U - D), 1.0);
    const float3 N = cross(B, T);

    // [0, 0] -> [-1, 0, 1]
    // [1, 0] -> [ 1, 0, 1]
    // [0, 1] -> [-1, 0,-1]
    // [1, 1] -> [ 1, 0,-1]
    // f(x,y) = (2x - 1, 0, 1.0 - 2y)
    output.texcoord = float2(x, y);
    output.posWorld = float3((2 * x - 1.0) * scaleX * 0.5, height * scaleY * 0.5, (1.0 - 2 * y) * scaleZ * 0.5);
    output.normalWorld = normalize(N);
    output.tangentWorld = normalize(T);
    output.posProj = mul(float4(output.posWorld, 1.0), viewProj);
    output.projTexcoord = mul(float4(output.posWorld, 1.0), projectionViewProj);

    return output;
}
