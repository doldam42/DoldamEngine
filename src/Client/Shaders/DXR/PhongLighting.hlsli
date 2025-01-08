#ifndef PHONG_LIGHTING_HLSL
#define PHONG_LIGHTING_HLSL

#include "common.hlsli"

float3 CalculateDiffuseLighting(float3 albedo, float3 hitPosition, float3 normal, Light light)
{
    float3 pixelToLight = normalize(light.position - hitPosition);
    
    // Diffuse contribution
    float NDotL = max(0.0, dot(pixelToLight, normal));
    
    return albedo * NDotL;
}

#endif // PHONG_LIGHTING_HLSL