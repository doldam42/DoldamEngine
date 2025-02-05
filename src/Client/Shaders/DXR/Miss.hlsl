#ifndef MISS_HLSL
#define MISS_HLSL

#include "RaytracingTypedef.hlsli"
#include "Common.hlsli"

[shader("miss")]
void Miss(inout HitInfo payload: SV_RayPayload)
{
    payload.colorAndDistance = float4(envIBLTex.SampleLevel(g_sampler, WorldRayDirection(), 0).xyz, HitDistanceOnMiss);
}

#endif // MISS_HLSL
