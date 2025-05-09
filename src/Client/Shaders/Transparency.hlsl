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
    float4x4 boneTransforms[64]; // 관절 개수에 따라 조절
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

// 메쉬 재질 텍스춰들 t0 부터 시작
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

    if (useNormalMap) // NormalWorld를 교체
    {
        float3 normal = normalTex.Sample(anisotropicWrapSampler, input.texcoord).rgb;

        normal = 2.0 * normal - 1.0; // 범위 조절 [-1.0, 1.0]

        // OpenGL 용 노멀맵일 경우에는 y 방향을 뒤집어줍니다.
        normal.y = -normal.y;
        // normal = normalize(normal);

        float3 N = normalWorld;
        float3 T = normalize(input.tangentWorld - dot(input.tangentWorld, N) * N);
        float3 B = cross(N, T);

        // matrix는 float4x4, 여기서는 벡터 변환용이라서 3x3 사용
        float3x3 TBN = float3x3(T, B, N);
        normalWorld = normalize(mul(normal, TBN));
    }

    return normalWorld;
}



// -------------------------------------