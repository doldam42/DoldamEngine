#include "Common.hlsli"

// 메쉬 재질 텍스춰들 t0 부터 시작
Texture2D albedoTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D aoTex : register(t2);
Texture2D metallicRoughnessTex : register(t3);
Texture2D emissiveTex : register(t4);

cbuffer cb : register(b5)
{
    uint useTexture;
    uint materialId;
    uint cbDummy[2];
};

float3 GetNormal(PixelShaderInput input, bool useNormalMap)
{
    float3 normalWorld = normalize(input.normalWorld);

    if (useNormalMap) // NormalWorld를 교체
    {
        float3 normal = normalTex.Sample(linearWrapSampler, input.texcoord).rgb;
        normal = 2.0 * normal - 1.0; // 범위 조절 [-1.0, 1.0]

        // OpenGL 용 노멀맵일 경우에는 y 방향을 뒤집어줍니다.
        // normal.y = -normal.y;

        float3 N = normalWorld;
        float3 T = normalize(input.tangentWorld - dot(input.tangentWorld, N) * N);
        float3 B = cross(N, T);

        // matrix는 float4x4, 여기서는 벡터 변환용이라서 3x3 사용
        float3x3 TBN = float3x3(T, B, N);
        normalWorld = normalize(mul(normal, TBN));
    }

    return normalWorld;
}

struct PS_OUTPUT
{
    float4 diffues : SV_Target0;
    float4 normal : SV_Target1;
    /*
     * r : reflect factor
     * g : roughness factor
     * b : metallic factor
     * a : transparancy factor
     */
    float4 elements : SV_Target2;
};

PS_OUTPUT main(PixelShaderInput input) 
{ 
    PS_OUTPUT output;
    const MaterialConstant material = g_materials[materialId];

    float4 texColor =
        material.useAlbedoMap ? albedoTex.Sample(linearWrapSampler, input.texcoord) : float4(material.albedo, 1.0);
    const float alpha = texColor.a * material.opacityFactor;
    if (alpha < 0.01)
    {
        discard;
    }

    float3 normalWorld = GetNormal(input, material.useNormalMap);
    
    float roughness = material.useRoughnessMap ? metallicRoughnessTex.Sample(linearWrapSampler, input.texcoord).g
                                               : material.roughnessFactor;
    float metallic = material.useMetallicMap ? metallicRoughnessTex.Sample(linearWrapSampler, input.texcoord).b
                                             : material.metallicFactor;

    output.diffues = float4(texColor.rgb, 1.0);
    output.normal = float4(normalWorld, 0.0);
    
    output.elements.r = material.reflectionFactor;
    output.elements.g = roughness;
    output.elements.b = metallic;
    output.elements.a = 1.0 - alpha;

    return output;
}