#include "Common.hlsli"
#include "Fragment.hlsli"

//////////////////////////////////////////////
// Defines
//////////////////////////////////////////////

static const float4x4 bias = float4x4(0.5, 0, 0, 0, 0, 0.5, 0, 0, 0, 0, 0.5, 0, 0.5, 0.5, 0.5, 1);

//////////////////////////////////////////////
// Structs
//////////////////////////////////////////////

//////////////////////////////////////////////
// Constant Buffers
//////////////////////////////////////////////

cbuffer MeshConstantBuffer : register(b1) { MeshConstant meshCB[256]; };

#ifdef SKINNED
cbuffer SkinnedConstantBuffer : register(b2)
{
    float4x4 boneTransforms[64]; // ���� ������ ���� ����
}
#endif

cbuffer cb : register(b5)
{
    uint materialId;
    uint useMaterial;
    uint useHeightMap;
    uint cbDummy[61];
};

//////////////////////////////////////////////
// Resources
//////////////////////////////////////////////

// �޽� ���� �ؽ���� t0 ���� ����
Texture2D albedoTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D aoTex : register(t2);
Texture2D metallicRoughnessTex : register(t3);
Texture2D emissiveTex : register(t4);


//////////////////////////////////////////////
// Helper Functions
//////////////////////////////////////////////

float3 GetNormal(PSInput input, bool useNormalMap)
{
    float3 normalWorld = normalize(input.normalWorld);

    if (useNormalMap) // NormalWorld�� ��ü
    {
        float3 normal = normalTex.Sample(anisotropicWrapSampler, input.texcoord).rgb;

        normal = 2.0 * normal - 1.0; // ���� ���� [-1.0, 1.0]

        // OpenGL �� ��ָ��� ��쿡�� y ������ �������ݴϴ�.
        normal.y = -normal.y;
        // normal = normalize(normal);

        float3 N = normalWorld;
        float3 T = normalize(input.tangentWorld - dot(input.tangentWorld, N) * N);
        float3 B = cross(N, T);

        // matrix�� float4x4, ���⼭�� ���� ��ȯ���̶� 3x3 ���
        float3x3 TBN = float3x3(T, B, N);
        normalWorld = normalize(mul(normal, TBN));
    }

    return normalWorld;
}



// -------------------------------------