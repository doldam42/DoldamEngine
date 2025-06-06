#include "Common.hlsli"

// 메쉬 재질 텍스춰들 t0 부터 시작
Texture2D albedoTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D aoTex : register(t2);
Texture2D metallicRoughnessTex : register(t3);
Texture2D emissiveTex : register(t4);

cbuffer cb : register(b5)
{
    uint materialId;
    uint useMaterial;
    uint useHeightMap;
    uint cbDummy[61];
};

float3 GetNormal(PSInput input, bool useNormalMap)
{
    float3 normalWorld = normalize(input.normalWorld);

    if (useNormalMap) // NormalWorld를 교체
    {
        float3 normal = normalTex.Sample(anisotropicWrapSampler, input.texcoord).rgb;

        normal = 2.0 * normal - 1.0; // 범위 조절 [-1.0, 1.0]

        // OpenGL 용 노멀맵일 경우에는 y 방향을 뒤집어줍니다.
        normal.y = -normal.y;
        //normal = normalize(normal);

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
    // a : opacity
    float4 diffues : SV_Target0;
    // a : emission
    float4 normal : SV_Target1;
    // r : ao factor
    // g : roughness factor
    // b : metallic factor
    // a : reflect factor
    float4 elements : SV_Target2;
};

PS_OUTPUT main(PSInput input) 
{ 
    PS_OUTPUT output;
    const MaterialConstant material = g_materials[materialId];

    float4 texColor = (material.flags & MATERIAL_USE_ALBEDO_MAP) ? albedoTex.Sample(linearWrapSampler, input.texcoord)
                                                                 : float4(material.albedo, 1.0);
    const float alpha = texColor.a * material.opacityFactor;
    clip(alpha - 0.001);

    float  ao = (material.flags & MATERIAL_USE_AO_MAP) ? aoTex.Sample(linearWrapSampler, input.texcoord).r : 1.0;
    float3 normalWorld = GetNormal(input, material.flags & MATERIAL_USE_NORMAL_MAP);
    
    float  roughness = (material.flags & MATERIAL_USE_ROUGHNESS_MAP)
                           ? metallicRoughnessTex.Sample(linearWrapSampler, input.texcoord).g
                                               : material.roughnessFactor;
    float  metallic = (material.flags & MATERIAL_USE_METALLIC_MAP)
                          ? metallicRoughnessTex.Sample(linearWrapSampler, input.texcoord).b
                                             : material.metallicFactor;
    float3 emission = (material.flags & MATERIAL_USE_EMISSIVE_MAP)
                          ? emissiveTex.Sample(linearWrapSampler, input.texcoord).rgb
                          : material.emissive;
    
    output.diffues = float4(texColor.rgb, alpha);
    output.normal = float4(normalWorld, length(emission));
    
    output.elements.r = ao;
    output.elements.g = roughness;
    output.elements.b = metallic;
    output.elements.a = material.reflectionFactor;

    return output;
}
