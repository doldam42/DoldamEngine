#include "Common.hlsli"
#include "PBR.hlsli"
#include "FragmentList.hlsli"


//--------------------------------------------------------------------------------------
// Resources
//--------------------------------------------------------------------------------------

// 메쉬 재질 텍스춰들 t0 부터 시작
Texture2D albedoTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D aoTex : register(t2);
Texture2D metallicRoughnessTex : register(t3);
Texture2D emissiveTex : register(t4);


//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

cbuffer cb : register(b5)
{
    uint materialId;
    uint useMaterial;
    uint useHeightMap;
    uint cbDummy[61];
};


//--------------------------------------------------------------------------------------
// Helper Functions
//--------------------------------------------------------------------------------------
float3 GetNormal(PSInput input, bool useNormalMap)
{
    float3 normalWorld = normalize(input.normalWorld);

    if (useNormalMap) // NormalWorld를 교체
    {
        float3 normal = normalTex.Sample(linearWrapSampler, input.texcoord).rgb;
        normal = 2.0 * normal - 1.0; // 범위 조절 [-1.0, 1.0]

        // OpenGL 용 노멀맵일 경우에는 y 방향을 뒤집어줍니다.
        normal.y = -normal.y;

        float3 N = normalWorld;
        float3 T = normalize(input.tangentWorld - dot(input.tangentWorld, N) * N);
        float3 B = cross(N, T);

        // matrix는 float4x4, 여기서는 벡터 변환용이라서 3x3 사용
        float3x3 TBN = float3x3(T, B, N);
        normalWorld = normalize(mul(normal, TBN));
    }

    return normalWorld;
}


//--------------------------------------------------------------------------------------
// Capture fragments phase
//--------------------------------------------------------------------------------------
[earlydepthstencil] 
void main(PSInput input, in uint coverageMask : SV_Coverage) 
{
    MaterialConstant material = g_materials[materialId];

    float3 pixelToEye = normalize(eyeWorld - input.posWorld);
    float3 normalWorld = GetNormal(input, material.flags & MATERIAL_USE_NORMAL_MAP);

    float4 texColor = (material.flags & MATERIAL_USE_ALBEDO_MAP) ? albedoTex.Sample(linearWrapSampler, input.texcoord)
                                                                 : float4(material.albedo, 1.0);
    if (useTextureProjection)
    {
        float2 projTexcoord;
        projTexcoord.x = input.projTexcoord.x / input.projTexcoord.w * 0.5 + 0.5;
        projTexcoord.y = -input.projTexcoord.y / input.projTexcoord.w * 0.5 + 0.5;
        if ((saturate(projTexcoord.x) == projTexcoord.x) && (saturate(projTexcoord.y) == projTexcoord.y))
        {
            texColor = projectionTex.Sample(linearWrapSampler, projTexcoord);
        }
    }

    
    float3 albedo = texColor.rgb;
    float  opacity = texColor.a * material.opacityFactor;
    
    float ao = (material.flags & MATERIAL_USE_AO_MAP) ? aoTex.Sample(linearWrapSampler, input.texcoord).r : 1.0;
    float roughness = (material.flags & MATERIAL_USE_ROUGHNESS_MAP)
                          ? metallicRoughnessTex.Sample(linearWrapSampler, input.texcoord).g
                          : material.roughnessFactor;
    float metallic = (material.flags & MATERIAL_USE_METALLIC_MAP)
                         ? metallicRoughnessTex.Sample(linearWrapSampler, input.texcoord).b
                         : material.metallicFactor;

    float3 emission = (material.flags & MATERIAL_USE_EMISSIVE_MAP)
                          ? emissiveTex.Sample(linearWrapSampler, input.texcoord).rgb
                          : material.emissive;

    float3 ambientLighting = AmbientLightingByIBL(albedo, normalWorld, pixelToEye, ao, metallic, roughness) * 0.2;

    float3 directLighting = 0;
    [unroll]
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        float3 L = lights[i].position - input.posWorld;
        float3 r = normalize(reflect(eyeWorld - input.posWorld, normalWorld));
        float3 centerToRay = dot(L, r) * r - L;
        float3 representativePoint = L + centerToRay * clamp(lights[i].radius / length(centerToRay), 0.0, 1.0);
        representativePoint += input.posWorld;
        float3 lightVec =
            (lights[i].type & LIGHT_DIRECTIONAL) ? -lights[i].direction : representativePoint - input.posWorld;

        float lightDist = length(lightVec);
        lightVec /= lightDist;

        float3 radiance = LightRadiance(lights[i], representativePoint, input.posWorld, normalWorld);

        directLighting += Shade(albedo, radiance, metallic, roughness, normalWorld, pixelToEye, lightVec);
    }

    float4 result = float4(ambientLighting + directLighting + emission, opacity);
    result = clamp(result, 0.0, 1000.0);

    float surfaceDepth = input.posView.z;

    // can't use discard here because it crashes ATI shader compiler
    if (0 != result.a)
    {
        // Get fragment viewport coordinates
        uint newNodeAddress;
        int2 screenAddress = int2(input.posProj.xy);

        if (FL_AllocNode(newNodeAddress))
        {
            // Fill node
            FragmentListNode node;
            node.color = FL_PackColor(result);
            node.depth = FL_PackDepthAndCoverage(surfaceDepth, coverageMask);

            // Insert node!
            FL_InsertNode(screenAddress, newNodeAddress, node);
        }
    }
}
