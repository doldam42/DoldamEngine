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
    output.posWorld = float3((2 * x - 1.0)*scaleX, 0, (1.0 - 2 * y)*scaleZ);

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
    const float height = input.height * 2.0 - 1.0;

    // [0, 0] -> [-1, 0, 1]
    // [1, 0] -> [ 1, 0, 1]
    // [0, 1] -> [-1, 0,-1]
    // [1, 1] -> [ 1, 0,-1]
    // f(x,y) = (2x - 1, 0, 1.0 - 2y)
    output.texcoord = float2(x, y);
    output.posWorld = float3((2 * x - 1.0) * scaleX, height * scaleY, (1.0 - 2 * y) * scaleZ);
    output.normalWorld = float3(0.0, 1.0, 0.0);
    output.tangentWorld = float3(1.0, 0.0, 0.0);
    output.posProj = mul(float4(output.posWorld, 1.0), viewProj);
    output.projTexcoord = mul(float4(output.posWorld, 1.0), projectionViewProj);

    return output;
}
