#include "Common.hlsli"

struct DepthOnlyPixelShaderInput
{
    float4 posProj : SV_Position;
};

float4 main(float4 pos : SV_Position) : SV_TARGET
{
    float farZ = 100.0;
    float depth = pos.z * pos.w / farZ;
    return float4(depth, depth, depth, 1.0);
}