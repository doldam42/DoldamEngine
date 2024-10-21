#include "Common.hlsli"

cbuffer MeshConstants : register(b1)
{
    float4x4 world;
    float4x4 worldIT;
    float4 mcDummy[8];
}

#ifdef SKINNED
cbuffer SkinnedConstantBuffer : register(b2)
{
    float4x4 boneTransforms[64]; // 관절 개수에 따라 조절
}
#endif

struct NormalGeometryShaderInput
{
    float4 posModel : SV_POSITION;
    float3 normalWorld : NORMAL;
};

struct NormalPixelShaderInput
{
    float4 pos : SV_POSITION;
    float3 color : COLOR;
};

static const float lineScale = 0.1;

NormalGeometryShaderInput VSMain(VertexShaderInput input)
{
    
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
    for(int i = 0; i < 4; ++i)
    {
        posModel += weights[i] * mul(float4(input.posModel, 1.0f), boneTransforms[indices[i]]).xyz;
        normalModel += weights[i] * mul(input.normalModel, (float3x3) boneTransforms[indices[i]]);
        tangentModel += weights[i] * mul(input.tangentModel, (float3x3) boneTransforms[indices[i]]);
    }

    input.posModel = posModel;
    input.normalModel = normalModel;
#endif
    
    NormalGeometryShaderInput output;
    
    output.posModel = float4(input.posModel, 1.0);
    output.normalWorld = input.normalModel;
    
    return output;
}

[maxvertexcount(2)]
void GSMain(point NormalGeometryShaderInput input[1], inout LineStream<NormalPixelShaderInput> outputStream)
{
    NormalPixelShaderInput output;
    float4 posWorld = mul(input[0].posModel, world);
    float4 normalModel = float4(input[0].normalWorld, 0.0);
    float4 normalWorld = mul(normalModel, worldIT);
    normalWorld = float4(normalize(normalWorld.xyz), 0.0);
    
    output.pos = mul(posWorld, viewProj);
    output.color = float3(1.0, 1.0, 0.0);
    outputStream.Append(output);
    
    output.pos = mul(posWorld + lineScale * normalWorld, viewProj);
    output.color = float3(1.0, 0.0, 0.0);
    outputStream.Append(output);
}

float4 PSMain(NormalPixelShaderInput input) : SV_Target
{
    return float4(input.color, 1.0f);
}