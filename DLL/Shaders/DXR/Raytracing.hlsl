#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

#include "Common.hlsli"
#include "PhongLighting.hlsli"

cbuffer l_RayFaceGroupConstants : register(b0, space1) { uint materialId; };

StructuredBuffer<Vertex> l_VB : register(t0, space1);
StructuredBuffer<uint>   l_IB : register(t1, space1);

Texture2D<float4> l_diffuseTex : register(t2, space1);
// Texture2D<float4> l_normalTex : register(t3, space1);

HitInfo TraceRadianceRay(in Ray ray, in uint currentRayRecursionDepth, float tMin = 0, float tMax = 100000,
                         float bounceContribution = 1, bool cullNonOpaque = false)
{
    HitInfo rayPayload;
    rayPayload.rayRecursionDepth = currentRayRecursionDepth + 1;

    if (currentRayRecursionDepth >= MAX_RADIENT_RAY_RECURSION_DEPTH)
    {
        rayPayload.colorAndDistance = float4(133 / 255.0, 161 / 255.0, 179 / 255.0, RayTCurrent());
        return rayPayload;
    }

    // Set the ray's extents.
    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;
    rayDesc.TMin = tMin;
    rayDesc.TMax = tMax;

    uint rayFlags = (cullNonOpaque ? RAY_FLAG_CULL_NON_OPAQUE : 0);

    TraceRay(
        // Parameter name: AccelerationStructure
        // Acceleration structure
        g_scene,

        // Parameter name: RayFlags
        // Flags can be used to specify the behavior upon hitting a surface
        rayFlags,

        // Parameter name: InstanceInclusionMask
        // Instance inclusion mask, which can be used to mask out some geometry to
        // this ray by and-ing the mask with a geometry mask. The 0xFF flag then
        // indicates no geometry will be masked
        INSTANCE_MASK,

        // Parameter name: RayContributionToHitGroupIndex
        // Depending on the type of ray, a given object can have several hit
        // groups attached (ie. what to do when hitting to compute regular
        // shading, and what to do when hitting to compute shadows). Those hit
        // groups are specified sequentially in the SBT, so the value below
        // indicates which offset (on 4 bits) to apply to the hit groups for this
        // ray. In this sample we only have one hit group per object, hence an
        // offset of 0.
        HITGROUP_INDEX_RADIENT,

        // Parameter name: MultiplierForGeometryContributionToHitGroupIndex
        // The offsets in the SBT can be computed from the object ID, its instance
        // ID, but also simply by the order the objects have been pushed in the
        // acceleration structure. This allows the application to group shaders in
        // the SBT in the same order as they are added in the AS, in which case
        // the value below represents the stride (4 bits representing the number
        // of hit groups) between two consecutive objects.
        GEOMETRY_STRIDE,

        // Parameter name: MissShaderIndex
        // Index of the miss shader to use in case several consecutive miss
        // shaders are present in the SBT. This allows to change the behavior of
        // the program when no geometry have been hit, for example one to return a
        // sky color for regular rendering, and another returning a full
        // visibility value for shadow rays. This sample has only one miss shader,
        // hence an index 0
        MISS_INDEX_RADIENT,

        // Parameter name: Ray
        // Ray information to trace
        rayDesc,

        // Parameter name: Payload
        // Payload associated to the ray, which will be used to communicate
        // between the hit/miss shaders and the raygen
        rayPayload);

    return rayPayload;
}

float3 TraceReflectiveRay(in float3 hitPosition, in float3 wi, in float3 N, in float3 objectNormal,
                          inout HitInfo rayPayload, in float TMax = 10000)
{
    // Here we offset ray start along the ray direction instead of surface normal
    // so that the reflected ray projects to the same screen pixel.
    // Offsetting by surface normal would result in incorrect mappating in temporally accumulated buffer.
    float  tOffset = 0.001f;
    float3 offsetAlongRay = tOffset * wi;

    float3 adjustedHitPosition = hitPosition + offsetAlongRay;

    Ray ray = {adjustedHitPosition, wi};

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

float3 Shade(inout HitInfo rayPayload, in float3 N, in float3 objectNormal, in float3 hitPosition,
             in MaterialConstant material)
{
    float3 V = -WorldRayDirection();
    bool   isReflective = (material.reflectionFactor > 1e-3);

    // Shadowing
    const Light  L = lights[0];
    const float3 lightPos = L.position;

    const float3 albedo = material.albedo;

    const float3 ambient = 0.2;
    const float3 diffuse = 0.2;
    const float3 specular = 1.0;

    float3 lightDir = normalize(lightPos - hitPosition);
    float  len = length(lightPos - hitPosition);

    // float3 color = CalculateDiffuseLighting(albedo, hitPosition, N, L).xyz;

    float3 color = albedo * ComputeDirectionalLight(L, N, V, ambient, diffuse, specular, 1.0);

    if (isReflective)
    {
        HitInfo reflectedRayPayLoad = rayPayload;
        float3  wi = reflect(-V, N);
        color += material.reflectionFactor * TraceReflectiveRay(hitPosition, wi, N, objectNormal, reflectedRayPayLoad);
    }

    bool  isInShadow = TraceShadowRayAndReportIfHit(hitPosition, lightDir, N, rayPayload, len);
    float shadowFactor = isInShadow ? 0.3 : 1.0;

    color *= shadowFactor;
    return color;
}

[shader("closesthit")] 
void ClosestHit(inout HitInfo payload, Attributes attrib) {
    uint        startIndex = PrimitiveIndex() * 3;
    const uint3 indices = {l_IB[startIndex], l_IB[startIndex + 1], l_IB[startIndex + 2]};
    Vertex      v[3] = {l_VB[indices[0]], l_VB[indices[1]], l_VB[indices[2]]};

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

    // float3 color = l_diffuseTex.SampleLevel(g_sampler, texcoord, 0).xyz;
    payload.colorAndDistance = float4(color, RayTCurrent());
}

[shader("raygeneration")] 
void RayGen()
{
    HitInfo payload;
    payload.colorAndDistance = float4(0, 0, 0, 0);
    payload.rayRecursionDepth = 0;

    uint2 launchIndex = DispatchRaysIndex().xy;

    Ray ray = GenerateCameraRay(launchIndex, eyeWorld, invViewProj);

    // Set the ray's extents.
    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;
    rayDesc.TMin = 0.0;
    rayDesc.TMax = 10000.0;

    uint rayFlags = RAY_FLAG_CULL_NON_OPAQUE;

    TraceRay(
        // Parameter name: AccelerationStructure
        // Acceleration structure
        g_scene,

        // Parameter name: RayFlags
        // Flags can be used to specify the behavior upon hitting a surface
        rayFlags,

        // Parameter name: InstanceInclusionMask
        // Instance inclusion mask, which can be used to mask out some geometry to
        // this ray by and-ing the mask with a geometry mask. The 0xFF flag then
        // indicates no geometry will be masked
        INSTANCE_MASK,

        // Parameter name: RayContributionToHitGroupIndex
        // Depending on the type of ray, a given object can have several hit
        // groups attached (ie. what to do when hitting to compute regular
        // shading, and what to do when hitting to compute shadows). Those hit
        // groups are specified sequentially in the SBT, so the value below
        // indicates which offset (on 4 bits) to apply to the hit groups for this
        // ray. In this sample we only have one hit group per object, hence an
        // offset of 0.
        HITGROUP_INDEX_RADIENT,

        // Parameter name: MultiplierForGeometryContributionToHitGroupIndex
        // The offsets in the SBT can be computed from the object ID, its instance
        // ID, but also simply by the order the objects have been pushed in the
        // acceleration structure. This allows the application to group shaders in
        // the SBT in the same order as they are added in the AS, in which case
        // the value below represents the stride (4 bits representing the number
        // of hit groups) between two consecutive objects.
        GEOMETRY_STRIDE,

        // Parameter name: MissShaderIndex
        // Index of the miss shader to use in case several consecutive miss
        // shaders are present in the SBT. This allows to change the behavior of
        // the program when no geometry have been hit, for example one to return a
        // sky color for regular rendering, and another returning a full
        // visibility value for shadow rays. This sample has only one miss shader,
        // hence an index 0
        MISS_INDEX_RADIENT,

        // Parameter name: Ray
        // Ray information to trace
        rayDesc,

        // Parameter name: Payload
        // Payload associated to the ray, which will be used to communicate
        // between the hit/miss shaders and the raygen
        payload);

    gOutput[launchIndex] = float4(payload.colorAndDistance.rgb, 1.0);
}

[shader("miss")] 
void Miss(inout HitInfo payload : SV_RayPayload) {
    uint2  launchIndex = DispatchRaysIndex().xy;
    float2 dims = float2(DispatchRaysDimensions().xy);

    float ramp = launchIndex.y / dims.y;
    payload.colorAndDistance = float4(0.0f, 0.2f, 0.7f - 0.3f * ramp, -1.0f);
}

[shader("closesthit")] 
void ShadowClosestHit(inout ShadowHitInfo hit, Attributes bary) { hit.tHit = RayTCurrent(); }

[shader("miss")] 
void ShadowMiss(inout ShadowHitInfo hit : SV_RayPayload)
{
    hit.tHit = 0;
}

#endif