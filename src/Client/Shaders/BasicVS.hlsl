#include "Common.hlsli"

cbuffer MeshConstantBuffer : register(b1)
{
    MeshConstant meshCB[256];
};

#ifdef SKINNED
cbuffer SkinnedConstantBuffer : register(b2)
{
    float4x4 boneTransforms[64]; // 관절 개수에 따라 조절
}
#endif

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;

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
    input.tangentModel = tangentModel;
#endif
    
    float4 pos = float4(input.posModel, 1.0f);
    pos = mul(pos, meshCB[input.instanceId].world);
    
    output.posWorld = pos.xyz;
   
    float4x4 positionMat =
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        -eyeWorld.x, -eyeWorld.y, -eyeWorld.z, 1
    };
    
    pos = mul(pos, positionMat);
    pos = mul(pos, viewProj);
    
    output.posProj = pos;
    output.texcoord = input.texcoord;
   
    float4 normal = float4(input.normalModel, 0.0f);
    output.normalWorld = mul(normal, meshCB[input.instanceId].worldIT).xyz;
    output.normalWorld = normalize(output.normalWorld);
    
    float4 tangent = float4(input.tangentModel, 0.0f);
    output.tangentWorld = mul(tangent, meshCB[input.instanceId].worldIT).xyz;
    output.tangentWorld = normalize(output.tangentWorld);
    
	return output;
}