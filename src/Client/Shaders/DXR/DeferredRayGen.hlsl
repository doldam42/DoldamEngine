#ifndef RAYGEN_HLSL
#define RAYGEN_HLSL

Texture2D<float4> diffuseTex : register(t1);
Texture2D<float4> normalTex : register(t2);
Texture2D<float4> elementsTex : register(t3);
Texture2D<float>  depthTex : register(t4);

#include "RaytracingTypedef.hlsli"
#include "HlslUtils.hlsli"
#include "Common.hlsli"

#include "BxDF.hlsli"


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

float3 TraceReflectiveRay(in float3 hitPosition, in float3 wi, in float3 N, 
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

float3 TraceRefractiveRay(in float3 hitPosition, in float3 wt, in float3 N, 
                          inout HitInfo rayPayload, in float TMax = 10000)
{
    // Here we offset ray start along the ray direction instead of surface normal
    // so that the reflected ray projects to the same screen pixel.
    // Offsetting by surface normal would result in incorrect mappating in temporally accumulated buffer.
    float  tOffset = 0.001f;
    float3 offsetAlongRay = tOffset * wt;

    float3 adjustedHitPosition = hitPosition + offsetAlongRay;

    Ray ray = {adjustedHitPosition, wt};

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
    float dummyTHit = 0.0;
    return TraceShadowRayAndReportIfHit(dummyTHit, visibilityRay, N, rayPayload.rayRecursionDepth, false,
                                        TMax); // TODO ASSERT
}

float3 Shade(inout HitInfo rayPayload, in float3 N, in float3 hitPosition,
             in MaterialConstant material)
{
    const float3 Fdielectric = 0.3;                    // 비금속(Dielectric) 재질의 F0
    const uint   materialType = MATERIAL_TYPE_DEFAULT; // 지금은 Default Material로 고정

    float3 V = normalize(eyeWorld - hitPosition);
    float3 L = 0;

    // Shadowing
    const float3 lightPos = lights[0].position;
    const float3 radiance = lights[0].radiance;
    const float3 lightDir = lights[0].direction;

    const float Kr = material.reflectionFactor;
    const float Kt = 1 - material.opacityFactor;
    const float metallic = material.metallicFactor;
    const float roughness = material.roughnessFactor;

    const float3 Kd = material.albedo;
    const float3 Ks = lerp(Fdielectric, Kd, metallic);

    // Direct illumination
    if (!BxDF::IsBlack(Kd) || !BxDF::IsBlack(Ks))
    {
        // float3 wi = normalize(lightPos - hitPosition);
        float3 wi = -lightDir;

        // Raytraced shadows.
        bool isInShadow = TraceShadowRayAndReportIfHit(hitPosition, wi, N, rayPayload);

        L += BxDF::DirectLighting::Shade(materialType, Kd, Ks, radiance, isInShadow, roughness, N, V, wi);
    }

    const float defaultAmbientIntensity = 0.04f;
    L += defaultAmbientIntensity * Kd;

    // Specular Indirect Illumination
    bool isReflective = !BxDF::IsBlack(Kr);
    bool isTransmissive = !BxDF::IsBlack(Kt);

    // Handle cases where ray is coming from behind due to imprecision,
    // don't cast reflection rays in that case.
    float smallValue = 1e-6f;
    isReflective = dot(V, N) > smallValue ? isReflective : false;

    if (isReflective || isTransmissive)
    {
        if (isReflective &&
            (BxDF::Specular::Reflection::IsTotalInternalReflection(V, N) || materialType == MATERIAL_TYPE_MIRROR))
        {
            float3 wi = reflect(-V, N);

            HitInfo reflectedRayPayLoad = rayPayload;
            L += Kr * TraceReflectiveRay(hitPosition, wi, N, reflectedRayPayLoad);
        }
        else // No total internal reflection
        {
            float3 Fo = Ks;
            if (isReflective)
            {
                // Radiance contribution from reflection.
                float3 wi;
                float3 Fr = Kr * BxDF::Specular::Reflection::Sample_Fr(V, wi, N, Fo); // Calculates wi

                HitInfo reflectedRayPayLoad = rayPayload;
                // Ref: eq 24.4, [Ray-tracing from the Ground Up]
                L += Fr * TraceReflectiveRay(hitPosition, wi, N, reflectedRayPayLoad);
            }

            if (isTransmissive)
            {
                // Radiance contribution from refraction.
                float3 wt;
                float3 Ft = Kt * BxDF::Specular::Transmission::Sample_Ft(V, wt, N, Fo); // Calculates wt

                HitInfo refractedRayPayLoad = rayPayload;
                L += Ft * TraceRefractiveRay(hitPosition, wt, N, refractedRayPayLoad);
            }
        }
    }

    return L;
}

[shader("raygeneration")] 
void DeferredRayGen() 
{
    HitInfo payload;
    payload.colorAndDistance = float4(0, 0, 0, 0);
    payload.rayRecursionDepth = 0;

    uint2 launchIndex = DispatchRaysIndex().xy;
    float2 xy = launchIndex + 0.5f;
    float2 screenCoord = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;
    // Invert Y for DirectX-style coordinate
    screenCoord.y = -screenCoord.y;

    float2 texcoord = ScreenToTextureCoord(screenCoord);

    float  depth = depthTex.SampleLevel(g_sampler, texcoord, 0).r;
    float3 posWorld = CalculateWorldPositionFromDepthMap(screenCoord, depth, invView, invProj);

    if (depth == 1.0)
    {
        Ray ray = GenerateCameraRay(launchIndex, eyeWorld, invViewProj);

        // Set the ray's extents.
        RayDesc rayDesc;
        rayDesc.Origin = ray.origin;
        rayDesc.Direction = ray.direction;
        rayDesc.TMin = 0.0;
        rayDesc.TMax = 10000.0;

        uint rayFlags = RAY_FLAG_CULL_NON_OPAQUE;

        TraceRay(g_scene, rayFlags, INSTANCE_MASK, HITGROUP_INDEX_RADIENT, GEOMETRY_STRIDE, MISS_INDEX_RADIENT, rayDesc,
                 payload);
        gOutput[launchIndex] = float4(payload.colorAndDistance.rgb, 1.0);
        return;
    }

    // Diffuse G-Buffer
    const float4 diffuse = diffuseTex.SampleLevel(g_sampler, texcoord, 0);
    const float3 albedo = diffuse.xyz;
    const float opacity = diffuse.w;

    // Normal G-Buffer
    const float4 N = normalTex.SampleLevel(g_sampler, texcoord, 0);
    const float3 normalWorld = N.xyz;
    const float3 emission = N.w;

    // Elements G-Buffer
    const float4 elements = elementsTex.SampleLevel(g_sampler, texcoord, 0);
    const float ao = elements.r;
    const float roughness = elements.g;
    const float metallic = elements.b;
    const float Kr = elements.a;

    MaterialConstant material;
    material.albedo = albedo;
    material.roughnessFactor = roughness;
    material.metallicFactor = metallic;
    material.reflectionFactor = Kr;
    material.opacityFactor = 1.0;  // TODO: transparancy

    float3 color = Shade(payload, normalWorld, posWorld, material);
     
    gOutput[launchIndex] = float4(color + emission, 1.0);
}

#endif