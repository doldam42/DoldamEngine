#include "RaytracingTypedef.h"
#include "BxDF.hlsli"
#include "Common.hlsli"

    struct RayGeomConstants
    {
        uint useTexture;
        uint materialId;
        float dummy[2];
    };

    ConstantBuffer<RayGeomConstants> l_materialCB : register(b0, space1);
    StructuredBuffer<Vertex> l_VB : register(t0, space1);
    StructuredBuffer<uint> l_IB : register(t1, space1);
    Texture2D<float4> l_diffuseTex : register(t2, space1);
//  Texture2D<float4> l_normalTex : register(t3, space1);

// Trace a shadow ray and return true if it hits any geometry.
    bool TraceShadowRayAndReportIfHit(out float tHit, in Ray ray, in uint currentRayRecursionDepth, in bool retrieveTHit = true, in float TMax = 10000)
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
        ShadowHitInfo shadowPayload = { TMax };

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

        TraceRay(g_scene,
        rayFlags,
        INSTANCE_MASK,
        HITGROUP_INDEX_SHADOW,
        GEOMETRY_STRIDE,
        MISS_INDEX_SHADOW,
        rayDesc, shadowPayload);
    
    // Report a hit if Miss Shader didn't set the value to HitDistanceOnMiss.
        tHit = shadowPayload.tHit;

        return shadowPayload.tHit > 0;
    }

    bool TraceShadowRayAndReportIfHit(out float tHit, in Ray ray, in float3 N, in uint currentRayRecursionDepth, in bool retrieveTHit = true, in float TMax = 10000)
    {
    // Only trace if the surface is facing the target.
        if (dot(ray.direction, N) > 0)
        {
            return TraceShadowRayAndReportIfHit(tHit, ray, currentRayRecursionDepth, retrieveTHit, TMax);
        }
        return false;
    }

    bool TraceShadowRayAndReportIfHit(in float3 hitPosition, in float3 direction, in float3 N, in HitInfo rayPayload, in float TMax = 10000)
    {
        float tOffset = 0.001f;
        Ray visibilityRay = { hitPosition + tOffset * N, direction };
        float dummyTHit;
        return TraceShadowRayAndReportIfHit(dummyTHit, visibilityRay, N, rayPayload.rayRecursionDepth, false, TMax); // TODO ASSERT
    }

    [shader("closesthit")]
void ClosestHit(inout HitInfo payload, Attributes attrib)
    {
        uint startIndex = PrimitiveIndex() * 3;
        const uint3 indices = { l_IB[startIndex], l_IB[startIndex + 1], l_IB[startIndex + 2] };
    
        Vertex v[3] =
        {
            l_VB[indices[0]],
        l_VB[indices[1]],
        l_VB[indices[2]]
        };
    
        float2 vertexTexCoords[3] = { v[0].texcoord, v[1].texcoord, v[2].texcoord };
        float2 texcoord = HitAttribute(vertexTexCoords, attrib);
    
        float3 vertexNormals[3] = { v[0].normalModel, v[1].normalModel, v[2].normalModel };
        float3 objectNormal = normalize(HitAttribute(vertexNormals, attrib));
        float orientation = HitKind() == HIT_KIND_TRIANGLE_FRONT_FACE ? 1 : -1;
        objectNormal *= orientation;
        float3 normal = normalize(mul((float3x3) ObjectToWorld3x4(), objectNormal));
    
        uint materialID = l_materialCB.materialId;
        MaterialConstant material = g_materials[materialID];
        
    // Shadowing
        float shadowFactor = 1.0;
        Light L = lights[1];
        float3 lightPos = L.position;
        
    // Find the world - space hit position
        float3 hitPosition = HitWorldPosition();
        
        float3 lightDir = normalize(lightPos - hitPosition);
        float len = length(lightPos - hitPosition);
    
        bool isInShadow = TraceShadowRayAndReportIfHit(hitPosition, lightDir, normal, payload, len);
        
        shadowFactor = isInShadow ? 0.3 : 1.0;
    
        float3 texSample = (l_materialCB.useTexture == 1) ? l_diffuseTex.SampleLevel(g_sampler, texcoord, 0).xyz : float3(0, 0, 0);
        float3 color = texSample * shadowFactor;
    //float3 texSample = cb.useTexture ? float3(1,1,0) : float3(0, 0, 0);
    
        payload.colorAndDistance = float4(color, RayTCurrent());
    }