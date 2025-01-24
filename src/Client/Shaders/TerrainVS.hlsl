#include "Common.hlsli"

cbuffer TerrainConstantBuffer : register(b1)
{
    uint  numSlice;
    uint  numStack;
    float scale;
    float heightScale;
    float tessFactor;
    uint  tcDummy[59];
}

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

    const int i = input.vId % (numSlice + 1);
    const int j = input.vId / (numSlice + 1);

    const float dx = 2.0f / numSlice;
    const float dz = 2.0f / numStack;

    const float x = -1.0f + dx * i;
    const float z = -1.0f + dz * j;
    const float height = input.height * 2.0 - 1.0;

    output.posWorld = float3(x * scale, height * heightScale, z * scale);
    output.texcoord = float2(1.0 + x, 1.0 - z) * 0.5;

    return output;
}

PSInput VSMainWithoutHS(VSCPInput input)
{
    PSInput output;

    const int i = input.vId % (numSlice + 1);
    const int j = input.vId / (numSlice + 1);

    const float dx = 2.0f / numSlice;
    const float dz = 2.0f / numStack;

    const float x = -1.0f + dx * i;
    const float z = -1.0f + dz * j;
    const float height = input.height * 2.0 - 1.0;

    output.posWorld = float3(x * scale, height * heightScale, z * scale);
    output.normalWorld = float3(0.0, 1.0, 0.0);
    output.texcoord = float2(1.0 + x, 1.0 - z) * 0.5;
    output.tangentWorld = float3(1.0, 0.0, 0.0);
    output.posProj = mul(float4(output.posWorld, 1.0), viewProj);
    output.projTexcoord = mul(float4(output.posWorld, 1.0), projectionViewProj);

    return output;
}
