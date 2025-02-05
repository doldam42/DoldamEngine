#include "RaytracingTypedef.hlsli"
#include "Common.hlsli"

[shader("closesthit")]
void ShadowClosestHit(inout ShadowHitInfo hit, Attributes bary)
{
    hit.tHit = RayTCurrent();
}

[shader("miss")]
void ShadowMiss(inout ShadowHitInfo hit : SV_RayPayload)
{
    hit.tHit = HitDistanceOnMiss;
}