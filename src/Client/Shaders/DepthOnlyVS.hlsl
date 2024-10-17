#include "Common.hlsli" // ���̴������� include ��� ����

cbuffer MeshConstantBuffer : register(b1)
{
    MeshConstant meshCB[256];
};

#ifdef SKINNED
cbuffer SkinnedConstantBuffer : register(b2)
{
    float4x4 boneTransforms[64]; // ���� ������ ���� ����
}
#endif

float4 main(VertexShaderInput input) : SV_POSITION
{
    
#ifdef SKINNED
    
    float weights[4];
    weights[0] = input.boneWeights.x;
    weights[1] = input.boneWeights.y;
    weights[2] = input.boneWeights.z;
    weights[3] = input.boneWeights.w;
    
    uint indices[4]; // ��Ʈ: �� ���!
    indices[0] = input.boneIndices.x;
    indices[1] = input.boneIndices.y;
    indices[2] = input.boneIndices.z;
    indices[3] = input.boneIndices.w;
    
    float3 posModel = float3(0.0f, 0.0f, 0.0f);
    float3 normalModel = float3(0.0f, 0.0f, 0.0f);
    float3 tangentModel = float3(0.0f, 0.0f, 0.0f);
    
    // Uniform Scaling ����
    // (float3x3)boneTransforms ĳ�������� Translation ����
    for(int i = 0; i < 4; ++i)
    {
        posModel += weights[i] * mul(float4(input.posModel, 1.0f), boneTransforms[indices[i]]).xyz;
        normalModel += weights[i] * mul(input.normalModel, (float3x3) boneTransforms[indices[i]]);
        tangentModel += weights[i] * mul(input.tangentModel, (float3x3) boneTransforms[indices[i]]);
    }

    input.posModel = posModel;
    input.normalModel = normalModel;
    input.tangentModel = tangentModel;
#endif
    
    float4 pos = mul(float4(input.posModel, 1.0f), meshCB[input.instanceId].world);
   
    pos = mul(float4(pos.xyz, 1.0), viewProj);
    
    return pos;
}