#include "Common.hlsli" // 쉐이더에서도 include 사용 가능

struct VertexShaderInput
{
    float3 posModel : POSITION;  // 모델 좌표계의 위치 position
    float3 normalModel : NORMAL; // 모델 좌표계의 normal
    float2 texcoord : TEXCOORD0;
    float3 tangentModel : TANGENT0;
    uint   instanceId : SV_InstanceID;

    float4 boneWeights : BLENDWEIGHT;
    uint4  boneIndices : BLENDINDICES;
};

cbuffer MeshConstantBuffer : register(b1) { MeshConstant meshCB[256]; };

cbuffer SkinnedConstantBuffer : register(b2)
{
    float4x4 boneTransforms[64]; // 관절 개수에 따라 조절
}

float4 main(VertexShaderInput input) : SV_POSITION
{
    float weights[4];
    weights[0] = input.boneWeights.x;
    weights[1] = input.boneWeights.y;
    weights[2] = input.boneWeights.z;
    weights[3] = input.boneWeights.w;

    uint indices[4]; 
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

    float4 pos = mul(float4(input.posModel, 1.0f), meshCB[input.instanceId].world);

    pos = mul(float4(pos.xyz, 1.0), viewProj);

    return pos;
}