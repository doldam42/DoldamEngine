#ifndef PHONG_LIGHTING_HLSL
#define PHONG_LIGHTING_HLSL

#include "common.hlsli"

float4 CalculateDiffuseLighting(float4 albedo, float3 hitPosition, float3 normal, Light light)
{
    float3 pixelToLight = normalize(light.position - hitPosition);
    
    // Diffuse contribution
    float NDotL = max(0.0, dot(pixelToLight, normal));
    
    return albedo * NDotL;
}

#endif // PHONG_LIGHTING_HLSL