#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

#include "Common.hlsli"
#include "HlslUtils.hlsli"
#include "RaytracingTypedef.hlsli"

#include "BxDF.hlsli"

cbuffer l_RayFaceGroupConstants : register(b0, space1)
{
    uint materialId;
    uint reserved;
};

Texture2D<float4> diffuseTex : register(t1);
Texture2D<float4> normalTex : register(t2);
Texture2D<float4> elementsTex : register(t3);
Texture2D<float>  depthTex : register(t4);

StructuredBuffer<Vertex> l_VB : register(t0, space1);
StructuredBuffer<uint>   l_IB : register(t1, space1);

Texture2D<float4> l_albedoTex : register(t2, space1);
Texture2D<float4> l_normalTex : register(t3, space1);
Texture2D<float4> l_AOTex : register(t4, space1);
Texture2D<float4> l_metallicRoughnessTex : register(t5, space1);
Texture2D<float4> l_emmisiveTex : register(t6, space1);

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

float3 TraceReflectiveRay(in float3 hitPosition, in float3 wi, in float3 N, inout HitInfo rayPayload,
                          in float TMax = 10000)
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

float3 TraceRefractiveRay(in float3 hitPosition, in float3 wt, in float3 N, inout HitInfo rayPayload,
                          in float TMax = 10000)
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

float3 Shade(inout HitInfo rayPayload, in float3 N, in float3 hitPosition, in MaterialConstant material)
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

float3 NormalMap(in float3 normal, in float2 texCoord, in Vertex vertices[3], in Attributes attr, in float lodLevel)
{
    float3 tangent;

    /*float3 v0 = vertices[0].posModel;
    float3 v1 = vertices[1].posModel;
    float3 v2 = vertices[2].posModel;
    float2 uv0 = vertices[0].texcoord;
    float2 uv1 = vertices[1].texcoord;
    float2 uv2 = vertices[2].texcoord;
    tangent = CalculateTangent(v0, v1, v2, uv0, uv1, uv2);*/

    float3 vertexTangents[3] = {vertices[0].tangentModel, vertices[1].tangentModel, vertices[2].tangentModel};
    tangent = HitAttribute(vertexTangents, attr);

    float3 texSample = l_normalTex.SampleLevel(g_sampler, texCoord, lodLevel).xyz;
    float3 bumpNormal = normalize(texSample * 2.f - 1.f);
    return BumpMapNormalToWorldSpaceNormal(bumpNormal, normal, tangent);
}

//***************************************************************************
//***********************------ Hit shaders -------**************************
//***************************************************************************
[shader("closesthit")] 
void ShadowClosestHit(inout ShadowHitInfo hit, Attributes bary) 
{ 
    hit.tHit = RayTCurrent(); 
}

[shader("closesthit")] 
void ClosestHit(inout HitInfo payload, Attributes attrib) {
    const static float LOG_FAR_PLANE = log(1.0 + FAR_PLANE);
    MaterialConstant   material = g_materials[materialId];

    // For LOD
    float distance = log(1.0 + RayTCurrent()) / LOG_FAR_PLANE; // log scale
    // float distance = RayTCurrent() / FAR_PLANE; // [0, 1]         // linear scale
    float lodLevel = lerp(0.0, 4.0, distance);
    // float lodLevel = 0;

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
    normal = (material.flags & MATERIAL_USE_NORMAL_MAP) ? NormalMap(normal, texcoord, v, attrib, lodLevel) : normal;

    material.roughnessFactor = (material.flags & MATERIAL_USE_ROUGHNESS_MAP)
                                   ? l_metallicRoughnessTex.SampleLevel(g_sampler, texcoord, lodLevel).g
                                   : material.roughnessFactor;
    material.metallicFactor = (material.flags & MATERIAL_USE_METALLIC_MAP)
                                  ? l_metallicRoughnessTex.SampleLevel(g_sampler, texcoord, lodLevel).b
                                  : material.metallicFactor;
    material.albedo = (material.flags & MATERIAL_USE_ALBEDO_MAP)
                          ? l_albedoTex.SampleLevel(g_sampler, texcoord, lodLevel).xyz
                          : material.albedo;
    // Find the world - space hit position
    float3 hitPosition = HitWorldPosition();

    float3 color = Shade(payload, normal, hitPosition, material);

    // float3 color = l_diffuseTex.SampleLevel(g_sampler, texcoord, 0).xyz;
    payload.colorAndDistance = float4(color, RayTCurrent());
}

//***************************************************************************
//***********************------ RayGen shaders -------**************************
//***************************************************************************

[shader("raygeneration")] 
void DeferredRayGen()
{
    HitInfo payload;
    payload.colorAndDistance = float4(0, 0, 0, 0);
    payload.rayRecursionDepth = 0;

    uint2  launchIndex = DispatchRaysIndex().xy;
    float2 xy = launchIndex + 0.5f;
    float2 screenCoord = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;
    // Invert Y for DirectX-style coordinate
    screenCoord.y = -screenCoord.y;

    float2 texcoord = ScreenToTextureCoord(screenCoord);

    float depth = depthTex.SampleLevel(g_sampler, texcoord, 0).r;
    if (depth == 1.0) // If Miss
    {
        const Ray ray = GenerateCameraRay(launchIndex, eyeWorld, invViewProj);
        gOutput[launchIndex] = float4(envIBLTex.SampleLevel(g_sampler, ray.direction, 0).xyz, 1.0);
        return;
    }

    float3 posWorld = CalculateWorldPositionFromDepthMap(screenCoord, depth, invView, invProj);
    float3 V = normalize(posWorld - eyeWorld);

    // Diffuse G-Buffer
    const float4 diffuse = diffuseTex.SampleLevel(g_sampler, texcoord, 0);
    const float3 albedo = diffuse.xyz;
    const float  opacity = diffuse.w;

    // Normal G-Buffer
    const float4 N = normalTex.SampleLevel(g_sampler, texcoord, 0);
    float3       normalWorld = N.xyz;
    if (dot(normalWorld, V) > 0)
        normalWorld *= -1;
    const float3 emission = N.w;

    // Elements G-Buffer
    const float4 elements = elementsTex.SampleLevel(g_sampler, texcoord, 0);
    const float  ao = elements.r;
    const float  roughness = elements.g;
    const float  metallic = elements.b;
    const float  Kr = elements.a;

    MaterialConstant material;
    material.albedo = albedo;
    material.roughnessFactor = roughness;
    material.metallicFactor = metallic;
    material.reflectionFactor = Kr;
    material.opacityFactor = 1.0; // TODO: transparancy

    // float3 color = Shade(payload, normalWorld, posWorld, material);
    float3 color = Shade(payload, normalWorld, posWorld, material);

    gOutput[launchIndex] = float4(color + emission, 1.0);
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

//***************************************************************************
//**********************------ Miss shaders -------**************************
//***************************************************************************

[shader("miss")] 
void ShadowMiss(inout ShadowHitInfo hit : SV_RayPayload)
{
    hit.tHit = HitDistanceOnMiss;
}

[shader("miss")] 
void Miss(inout HitInfo payload : SV_RayPayload)
{
    payload.colorAndDistance = float4(envIBLTex.SampleLevel(g_sampler, WorldRayDirection(), 0).xyz, HitDistanceOnMiss);
}

#endif // HIT_HLSL