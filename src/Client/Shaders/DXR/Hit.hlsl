#ifndef HIT_HLSL
#define HIT_HLSL

#include "Common.hlsli"
#include "PhongLighting.hlsli"
//#include "RaytracingTypedef.h"

cbuffer l_RayFaceGroupConstants : register(b0, space1)
{
    uint  materialId;
};

StructuredBuffer<Vertex>         l_VB : register(t0, space1);
StructuredBuffer<uint>           l_IB : register(t1, space1);
Texture2D<float4>                l_diffuseTex : register(t2, space1);
// Texture2D<float4> l_normalTex : register(t3, space1);

HitInfo TraceRadianceRay(Ray ray, in uint currentRayRecursionDepth, float tMin = NEAR_PLANE, float tMax = FAR_PLANE, float bounceContribution = 1, bool cullNonOpaque = false)
{
    HitInfo rayPayload;
    rayPayload.colorAndDistance = float4(0, 0, 0, 0);
    rayPayload.rayRecursionDepth = currentRayRecursionDepth + 1;
    
    if (currentRayRecursionDepth >= MAX_RAY_RECURSION_DEPTH)
    {
        rayPayload.colorAndDistance = float4(133/255.0, 161/255.0, 179/255.0, 0);
        return rayPayload;
    }

    // Set the ray's extents.
    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;
    rayDesc.TMin = tMin;
    rayDesc.TMax = tMax;

    uint rayFlags = (cullNonOpaque ? RAY_FLAG_CULL_NON_OPAQUE : 0);

    TraceRay(g_scene,
        rayFlags,
		INSTANCE_MASK, HITGROUP_INDEX_RADIENT, GEOMETRY_STRIDE, MISS_INDEX_RADIENT,
		rayDesc, rayPayload);

    return rayPayload;
}

float3 TraceReflectiveRay(in float3 hitPosition, in float3 wt, in float3 N, in float3 objectNormal, inout HitInfo rayPayload, in float TMax = 10000)
{
    // Here we offset ray start along the ray direction instead of surface normal 
    // so that the reflected ray projects to the same screen pixel. 
    // Offsetting by surface normal would result in incorrect mappating in temporally accumulated buffer. 
    float tOffset = 0.001f;
    float3 offsetAlongRay = tOffset * wt;

    float3 adjustedHitPosition = hitPosition + offsetAlongRay;

    Ray ray = { adjustedHitPosition, wt };

    float tMin = 0;
    float tMax = TMax;

    // TRADEOFF: Performance vs visual quality
    // Cull transparent surfaces when casting a transmission ray for a transparent surface.
    // Spaceship in particular has multiple layer glass causing a substantial perf hit 
    // with multiple bounces along the way.
    // This can cause visual pop ins however, such as in a case of looking at the spaceship's
    // glass cockpit through a window in the house. The cockpit will be skipped in this case.
    bool cullNonOpaque = true;

    rayPayload = TraceRadianceRay(ray, rayPayload.rayRecursionDepth, tMin, tMax, 0, cullNonOpaque);
    
    if (rayPayload.colorAndDistance.w != HitDistanceOnMiss)
    {
        // Add current thit and the added offset to the thit of the traced ray.
        rayPayload.colorAndDistance.w += RayTCurrent() + tOffset;
    }

    return rayPayload.colorAndDistance.xyz;
}

// Trace a shadow ray and return true if it hits any geometry.
bool TraceShadowRayAndReportIfHit(out float tHit, in Ray ray, in uint currentRayRecursionDepth,
                                  in bool retrieveTHit = true, in float TMax = FAR_PLANE)
{
    if (currentRayRecursionDepth >= 3)
    {
        return false;
    }

    // Set the ray's extents.
    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;
    rayDesc.TMin = 0.0;
    rayDesc.TMax = TMax;

    // Initialize shadow ray payload.
    // Set the initial value to a hit at TMax.
    // Miss shader will set it to HitDistanceOnMiss.
    // This way closest and any hit shaders can be skipped if true tHit is not needed.
    ShadowHitInfo shadowPayload = {TMax};

    uint rayFlags = RAY_FLAG_CULL_NON_OPAQUE; // ~skip transparent objects
    bool acceptFirstHit = !retrieveTHit;
    if (acceptFirstHit)
    {
        // Performance TIP: Accept first hit if true hit is not neeeded,
        // or has minimal to no impact. The peformance gain can
        // be substantial.
        rayFlags |= RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH;
    }

    // Skip closest hit shaders of tHit time is not needed.
    if (!retrieveTHit)
    {
        rayFlags |= RAY_FLAG_SKIP_CLOSEST_HIT_SHADER;
    }
    
    TraceRay(g_scene, rayFlags, INSTANCE_MASK, HITGROUP_INDEX_SHADOW, GEOMETRY_STRIDE, MISS_INDEX_SHADOW, rayDesc,
             shadowPayload);

    // Report a hit if Miss Shader didn't set the value to HitDistanceOnMiss.
    tHit = shadowPayload.tHit;

    return shadowPayload.tHit > 0;
}

bool TraceShadowRayAndReportIfHit(out float tHit, in Ray ray, in float3 N, in uint currentRayRecursionDepth,
                                  in bool retrieveTHit = true, in float TMax = 10000)
{
    // Only trace if the surface is facing the target.
    if (dot(ray.direction, N) > 0)
    {
        return TraceShadowRayAndReportIfHit(tHit, ray, currentRayRecursionDepth, retrieveTHit, TMax);
    }
    return false;
}

bool TraceShadowRayAndReportIfHit(in float3 hitPosition, in float3 direction, in float3 N, in HitInfo rayPayload,
                                  in float TMax = 10000)
{
    float tOffset = 0.001f;
    Ray   visibilityRay = {hitPosition + tOffset * N, direction};
    float dummyTHit;
    return TraceShadowRayAndReportIfHit(dummyTHit, visibilityRay, N, rayPayload.rayRecursionDepth, false,
                                        TMax); // TODO ASSERT
}

float3 Shade(inout HitInfo rayPayload, in float3 N, in float3 objectNormal, in float3 hitPosition, in MaterialConstant material)
{
    bool isReflective = (material.reflectionFactor > 1e-3);
    
    // Shadowing
    const Light L = lights[0];
    const float3 lightPos = L.position;
    
    const float3 albedo = material.albedo;
    
    float3 lightDir = normalize(lightPos - hitPosition);
    float len = length(lightPos - hitPosition);
    
    float3 color = CalculateDiffuseLighting(albedo, hitPosition, N, L).xyz;

    bool isInShadow = TraceShadowRayAndReportIfHit(hitPosition, lightDir, N, rayPayload, len);
    float shadowFactor = isInShadow ? 0.3 : 1.0;

    color *= shadowFactor;
    return color;
}

[shader("closesthit")] 
void ClosestHit(inout HitInfo payload, Attributes attrib) {
    uint        startIndex = PrimitiveIndex() * 3;
    const uint3 indices = {l_IB[startIndex], l_IB[startIndex + 1], l_IB[startIndex + 2]};

    Vertex v[3] = {l_VB[indices[0]], l_VB[indices[1]], l_VB[indices[2]]};

    float2 vertexTexCoords[3] = {v[0].texcoord, v[1].texcoord, v[2].texcoord};
    float2 texcoord = HitAttribute(vertexTexCoords, attrib);

    float3 vertexNormals[3] = {v[0].normalModel, v[1].normalModel, v[2].normalModel};
    float3 objectNormal = normalize(HitAttribute(vertexNormals, attrib));
    float  orientation = HitKind() == HIT_KIND_TRIANGLE_FRONT_FACE ? 1 : -1;
    objectNormal *= orientation;
    float3 normal = normalize(mul((float3x3)ObjectToWorld3x4(), objectNormal));
    
    MaterialConstant material = g_materials[materialId];
    
    material.albedo =
        (material.useAlbedoMap == TRUE) ? l_diffuseTex.SampleLevel(g_sampler, texcoord, 0).xyz : material.albedo;
    
    // Find the world - space hit position
    float3 hitPosition = HitWorldPosition();
    
    float3 color = Shade(payload, normal, objectNormal, hitPosition, material);

    payload.colorAndDistance = float4(color, RayTCurrent());
}

#endif // HIT_HLSL