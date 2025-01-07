// Hit information, aka ray payload
// This sample only carries a shading color and hit distance.
// Note that the payload should be kept as small as possible,
// and that its size must be declared in the corresponding
// D3D12_RAYTRACING_SHADER_CONFIG pipeline subobjet.

#ifndef COMMON_HLSL
#define COMMON_HLSL

#define TRUE 1
#define FALSE 0

#define MAX_RAY_RECURSION_DEPTH 2
#define MAX_SHADOW_RAY_RECURSION_DEPTH 1

#define INSTANCE_MASK ~0
#define GEOMETRY_STRIDE 2

#define MISS_INDEX_RADIENT 0
#define MISS_INDEX_SHADOW 1

#define HITGROUP_INDEX_RADIENT 0
#define HITGROUP_INDEX_SHADOW 1
#define HITGROUP_INDEX_COUNT 2

// TODO: Add recursion Depth
struct HitInfo
{
    uint rayRecursionDepth;
    float4 colorAndDistance;
};

struct ShadowHitInfo
{
    float tHit;
};

struct Vertex
{
    float3 posModel; //¸ðµ¨ ÁÂÇ¥°èÀÇ À§Ä¡ position
    float3 normalModel; // ¸ðµ¨ ÁÂÇ¥°èÀÇ normal    
    float2 texcoord;
    float3 tangentModel;
};

struct MaterialConstant
{
    float3 albedo;
    float roughnessFactor;
    float metallicFactor;
    float3 emissive;
    
    float opacityFactor;
    float reflectionFactor;
    
    uint useAlbedoMap;
    uint useNormalMap;
    uint useAOMap;
    uint useMetallicMap;
    uint useRoughnessMap;
    uint useEmissiveMap;
};

// Attributes output by the raytracing when hitting a surface,
// here the barycentric coordinates
struct Attributes
{
    float2 bary;
};

struct Ray
{
    float3 origin;
    float3 direction;
};

inline Ray GenerateCameraRay(in uint2 index, in float3 cameraPosition, in float4x4 cameraToWorld)
{
    float2 xy = index + 0.5f;
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;
    
    // Invert Y for DirectX-style coordinate
    screenPos.y = -screenPos.y;
    
    float4 unprojected = mul(float4(screenPos, 0, 1), cameraToWorld);
    float3 world = unprojected.xyz / unprojected.w;
    
    Ray ray;
    ray.origin = cameraPosition;
    
    ray.direction = normalize(world - cameraPosition);
    
    return ray;
}

float2 HitAttribute(float2 vertexAttribute[3], Attributes attr)
{
    return vertexAttribute[0] +
        attr.bary.x * (vertexAttribute[1] - vertexAttribute[0]) +
        attr.bary.y * (vertexAttribute[2] - vertexAttribute[0]);
}

float3 HitAttribute(float3 vertexAttribute[3], Attributes attr)
{
    return vertexAttribute[0] +
        attr.bary.x * (vertexAttribute[1] - vertexAttribute[0]) +
        attr.bary.y * (vertexAttribute[2] - vertexAttribute[0]);
}

float3 HitWorldPosition()
{
    return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

#define MAX_LIGHTS 3

struct Light
{
    float3 strength;
    float fallOffStart;
    float3 direction;
    float fallOffEnd;
    float3 position;
    float spotPower;
};

cbuffer g_cb : register(b0)
{
    float4x4 view;
    float4x4 invView;
    float4x4 proj;
    float4x4 invProj;
    float4x4 viewProj;
    float4x4 invViewProj; // Proj -> World
    float4x4 projectionViewProj; // texture projection 
    
    float3 eyeWorld;
    float strengthIBL;
    
    Light lights[MAX_LIGHTS];
    uint useTextureProjection;
    float gcDummy2[59];
}
// Raytracing output texture, accessed as a UAV
RWTexture2D<float4> gOutput : register(u0);
// Raytracing acceleration structure, accessed as a SRV
RaytracingAccelerationStructure g_scene : register(t0);
SamplerState g_sampler : register(s0);
StructuredBuffer<MaterialConstant> g_materials : register(t5);

#endif  // COMMON_HLSL