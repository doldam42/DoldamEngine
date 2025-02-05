#ifndef RAYGEN_HLSL
#define RAYGEN_HLSL

#include "RaytracingTypedef.hlsli"
#include "Common.hlsli"

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

#endif