struct BasicVertex
{
    float3 posModel; //모델 좌표계의 위치 position
    float3 normalModel; // 모델 좌표계의 normal    
    float2 texcoord;
    float3 tangentModel;
};

struct SkinnedVertex
{
    float3 posModel; //모델 좌표계의 위치 position
    float3 normalModel; // 모델 좌표계의 normal    
    float2 texcoord;
    float3 tangentModel;
    float4 boneWeights;
    uint boneIndices; // 8bit * 4
};

cbuffer SkinnedConstantBuffer : register(b0)
{
    float4x4 boneTransforms[64]; // 관절 개수에 따라 조절
}
StructuredBuffer<SkinnedVertex> inputVertice : register(t0);
RWStructuredBuffer<BasicVertex> outputVertice : register(u0);

[numthreads(1024, 1, 1)]
void main(int3 gID : SV_GroupID, uint3 DTid : SV_DispatchThreadID)
{
    uint id = DTid.x;
    SkinnedVertex v = inputVertice[id];
    
    float weights[4];
    weights[0] = v.boneWeights.x;
    weights[1] = v.boneWeights.y;
    weights[2] = v.boneWeights.z;
    weights[3] = v.boneWeights.w;
    
    // vertex.boneIndices.
    uint indices[4];
    // 리틀 엔디안 
    indices[0] = (v.boneIndices & 0x000000ff);
    indices[1] = (v.boneIndices & 0x0000ff00) >> 8;
    indices[2] = (v.boneIndices & 0x00ff0000) >> 16;
    indices[3] = v.boneIndices >> 24;
    
    float3 posModel = float3(0.0f, 0.0f, 0.0f);
    float3 normalModel = float3(0.0f, 0.0f, 0.0f);
    float3 tangentModel = float3(0.0f, 0.0f, 0.0f);
    
    // Uniform Scaling 가정
    // (float3x3)boneTransforms 캐스팅으로 Translation 제외
    for (int i = 0; i < 4; ++i)
    {
        posModel += weights[i] * mul(float4(v.posModel, 1.0f), boneTransforms[indices[i]]).xyz;
        normalModel += weights[i] * mul(v.normalModel, (float3x3) boneTransforms[indices[i]]);
        tangentModel += weights[i] * mul(v.tangentModel, (float3x3) boneTransforms[indices[i]]);
    }

    outputVertice[id].posModel = posModel;
    outputVertice[id].normalModel = normalModel;
    outputVertice[id].texcoord = v.texcoord;
    outputVertice[id].tangentModel = tangentModel;
}