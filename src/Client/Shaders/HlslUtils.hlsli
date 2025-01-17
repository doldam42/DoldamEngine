// Retrieves pixel's position in world space.
float3 CalculateWorldPositionFromDepthMap(float2 screenCoord, float4x4 invView, float4x4 invProj, float depth)
{
    // Translate from homogeneous coords to texture coords.
    float2 depthTexCoord = 0.5f * screenCoord + 0.5f;
    depthTexCoord.y = 1.0f - depthTexCoord.y;

    float4 screenPos = float4(screenCoord.x, screenCoord.y, depth, 1.0);
    float4 viewPosition = mul(screenPos, invProj);
    viewPosition /= viewPosition.w; // Perspective division
    float4 worldPosition = mul(viewPosition, invView);

    return worldPosition.xyz;
}