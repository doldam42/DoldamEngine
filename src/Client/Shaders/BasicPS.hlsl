#include "Common.hlsli"
#include "Phong.hlsli"

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

static const float3 Fdielectric = 0.04; // 비금속(Dielectric) 재질의 F0
static const float  lod = 2.0;

float3 SchlickFresnel(float3 F0, float NdotH)
{
    return F0 + (1.0 - F0) * pow(2.0, (-5.55473 * NdotH - 6.98316) * NdotH);
    // return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

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

float3 DiffuseIBL(float3 albedo, float3 normalWorld, float3 pixelToEye, float metallic)
{
    float3 F0 = lerp(Fdielectric, albedo, metallic);
    float3 F = SchlickFresnel(F0, max(0.0, dot(normalWorld, pixelToEye)));
    float3 kd = lerp(1.0 - F, 0.0, metallic);
    float3 irradiance = irradianceIBLTex.SampleLevel(linearWrapSampler, normalWorld, 0).rgb;

    return kd * albedo * irradiance;
}

float3 SpecularIBL(float3 albedo, float3 normalWorld, float3 pixelToEye, float metallic, float roughness)
{
    float2 specularBRDF =
        brdfTex.SampleLevel(linearClampSampler, float2(dot(normalWorld, pixelToEye), 1.0 - roughness), 0.0f).rg;
    float3 specularIrradiance =
        specularIBLTex.SampleLevel(linearWrapSampler, reflect(-pixelToEye, normalWorld), 2 + roughness * 5.0f).rgb;
    const float3 Fdielectric = 0.04; // 비금속(Dielectric) 재질의 F0
    float3       F0 = lerp(Fdielectric, albedo, metallic);

    return (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;
}

float3 AmbientLightingByIBL(float3 albedo, float3 normalW, float3 pixelToEye, float ao, float metallic, float roughness)
{
    float3 diffuseIBL = DiffuseIBL(albedo, normalW, pixelToEye, metallic);
    float3 specularIBL = SpecularIBL(albedo, normalW, pixelToEye, metallic, roughness);

    return (diffuseIBL + specularIBL) * ao;
}

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float NdfGGX(float NdotH, float roughness)
{
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;
    float denom = (NdotH * NdotH) * (alphaSq - 1.0) + 1.0;

    return alphaSq / (3.141592 * denom * denom);
}

// Single term for separable Schlick-GGX below.
float SchlickG1(float NdotV, float k) { return NdotV / (NdotV * (1.0 - k) + k); }

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float SchlickGGX(float NdotI, float NdotO, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return SchlickG1(NdotI, k) * SchlickG1(NdotO, k);
}

float3 LightRadiance(Light light, float3 representativePoint, float3 posWorld, float3 normalWorld, Texture2D shadowMap)
{
    // Directional light
    float3 lightVec = light.type & LIGHT_DIRECTIONAL ? -light.direction
                                                     : representativePoint - posWorld; //: light.position - posWorld;

    float lightDist = length(lightVec);
    lightVec /= lightDist;

    // Spot light
    float spotFator = light.type & LIGHT_SPOT ? pow(max(-dot(lightVec, light.direction), 0.0f), light.spotPower) : 1.0f;

    // Distance attenuation
    float att = saturate((light.fallOffEnd - lightDist) / (light.fallOffEnd - light.fallOffStart));

    // Shadow Map
    float shadowFactor = 1.0;

    if (light.type & LIGHT_SHADOW)
    {
        const float nearZ = 0.01;

        // 1. Project posWorld to light screen
        float4 lightScreen = mul(float4(posWorld, 1.0), light.viewProj);
        lightScreen.xyz /= lightScreen.w;

        // 2. 카메라에서 볼 때의 텍스쳐 좌표 계산
        float2 lightTexcoord = float2(lightScreen.x, -lightScreen.y);
        lightTexcoord += 1.0;
        lightTexcoord *= 0.5;

        // 3. 셰도우 맵에서 값 가져오기
        float depth = shadowMap.Sample(shadowPointSampler, lightTexcoord).r;

        if (depth + 1e-3 < lightScreen.z)
            shadowFactor = 0.0; // <- 0.0의 의미는?
    }

    float3 radiance = light.radiance * spotFator * att * shadowFactor;

    return radiance;
}

float3 LightRadiance(Light light, float3 representativePoint, float3 posWorld, float3 normalWorld)
{
    // Directional light
    float3 lightVec = light.type & LIGHT_DIRECTIONAL ? -light.direction
                                                     : representativePoint - posWorld; //: light.position - posWorld;

    float lightDist = length(lightVec);
    lightVec /= lightDist;

    // Spot light
    float spotFator = light.type & LIGHT_SPOT ? pow(max(-dot(lightVec, light.direction), 0.0f), light.spotPower) : 1.0f;

    // Distance attenuation
    float att = saturate((light.fallOffEnd - lightDist) / (light.fallOffEnd - light.fallOffStart));

    float3 radiance = light.radiance * spotFator * att;

    return radiance;
}

float4 main(PSInput input) : SV_TARGET
{   
    MaterialConstant material = g_materials[materialId];

    float3 pixelToEye = normalize(eyeWorld - input.posWorld);
    float3 normalWorld = GetNormal(input, material.flags & MATERIAL_USE_NORMAL_MAP);

    float4 texColor = (material.flags & MATERIAL_USE_ALBEDO_MAP)
                          ? albedoTex.Sample(linearWrapSampler, input.texcoord)
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

    if (opacity < 0.05)
    {
        discard;
    }

    float ao = (material.flags & MATERIAL_USE_AO_MAP) ? aoTex.Sample(linearWrapSampler, input.texcoord).r : 1.0;
    float roughness = (material.flags & MATERIAL_USE_ROUGHNESS_MAP)
                          ? metallicRoughnessTex.Sample(linearWrapSampler, input.texcoord).g
                                               : material.roughnessFactor;
    float metallic = (material.flags & MATERIAL_USE_METALLIC_MAP)
                         ? metallicRoughnessTex.Sample(linearWrapSampler, input.texcoord).b
                                             : material.metallicFactor;

    float3 emission =
        (material.flags & MATERIAL_USE_EMISSIVE_MAP) ? emissiveTex.Sample(linearWrapSampler, input.texcoord).rgb : material.emissive;
    
    float3 ambientLighting = AmbientLightingByIBL(albedo, normalWorld, pixelToEye, ao, metallic, roughness) * 0.2;

    float3 directLighting = float3(0, 0, 0);
    [unroll]
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].type != LIGHT_OFF)
        {
            float3 L = lights[i].position - input.posWorld;
            float3 r = normalize(reflect(eyeWorld - input.posWorld, normalWorld));
            float3 centerToRay = dot(L, r) * r - L;
            float3 representativePoint = L + centerToRay * clamp(lights[i].radius / length(centerToRay), 0.0, 1.0);
            representativePoint += input.posWorld;
            float3 lightVec =
                (lights[i].type & LIGHT_DIRECTIONAL) ? -lights[i].direction : representativePoint - input.posWorld;

            // float3 lightVec = lights[i].position - input.posWorld;
            float lightDist = length(lightVec);
            lightVec /= lightDist;
            float3 halfway = normalize(pixelToEye + lightVec);

            float NdotI = max(0.0, dot(normalWorld, lightVec));
            float NdotH = max(0.0, dot(normalWorld, halfway));
            float NdotO = max(0.0, dot(normalWorld, pixelToEye));

            const float3 Fdielectric = 0.04; // 비금속(Dielectric) 재질의 F0
            float3 F0 = lerp(Fdielectric, albedo, metallic);
            float3 F = SchlickFresnel(F0, max(0.0, dot(halfway, pixelToEye)));
            float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metallic);
            float3 diffuseBRDF = kd * albedo;

            float D = NdfGGX(NdotH, roughness);
            float3 G = SchlickGGX(NdotI, NdotO, roughness);
            float3 specularBRDF = (F * D * G) / max(1e-5, 4.0 * NdotI * NdotO);

            float3 radiance = LightRadiance(lights[i], input.posWorld, input.posWorld, normalWorld);

            directLighting += (diffuseBRDF + specularBRDF) * radiance * NdotI;
        }
    }

    float4 output;
    output = float4(ambientLighting + directLighting + emission, opacity);
    output = clamp(output, 0.0, 1000.0);
    return output;
}