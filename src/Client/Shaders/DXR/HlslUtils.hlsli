// Calculate a tangent from triangle's vertices and their uv coordinates.
// Ref: http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/
float3 CalculateTangent(in float3 v0, in float3 v1, in float3 v2, in float2 uv0, in float2 uv1, in float2 uv2)
{
    // Calculate edges
    // Position delta
    float3 deltaPos1 = v1 - v0;
    float3 deltaPos2 = v2 - v0;

    // UV delta
    float2 deltaUV1 = uv1 - uv0;
    float2 deltaUV2 = uv2 - uv0;

    float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
    return (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
}

// Sample normal map, convert to signed, apply tangent-to-world space transform.
float3 BumpMapNormalToWorldSpaceNormal(float3 bumpNormal, float3 surfaceNormal, float3 tangent)
{
    // Compute tangent frame.
    surfaceNormal = normalize(surfaceNormal);
    tangent = normalize(tangent);

    float3 bitangent = normalize(cross(tangent, surfaceNormal));
    float3x3 tangentSpaceToWorldSpace = float3x3(tangent, bitangent, surfaceNormal);

    return mul(bumpNormal, tangentSpaceToWorldSpace);
}

// Retrieves pixel's position in world space.
float3 CalculateWorldPositionFromDepthMap(float2 screenCoord, float depth, float4x4 invView, float4x4 invProj)
{
    float4 screenPos = float4(screenCoord.x, screenCoord.y, depth, 1.0);
    float4 viewPosition = mul(screenPos, invProj);
    viewPosition /= viewPosition.w; // Perspective division
    float4 worldPosition = mul(viewPosition, invView);

    return worldPosition.xyz;
}

inline float2 ScreenToTextureCoord(float2 screenCoord)
{
    float2 texcoord = float2(screenCoord.x, -screenCoord.y);
    texcoord += 1.0;
    texcoord *= 0.5;
    return texcoord;
}

inline float2 TextureToScreenCoord(float2 texcoord)
{
    float2 screenCoord = 2 * texcoord - 1;
    screenCoord.y = -screenCoord.y;
    return screenCoord;
}