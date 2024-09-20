#include "Common.hlsli"

struct SkyboxPixelShaderInput
{
    float4 posProj : SV_POSITION;
    float3 posModel : POSITION;
};

float4 main(SkyboxPixelShaderInput input) : SV_TARGET
{   
    float3 color = envIBLTex.SampleLevel(linearWrapSampler,
    input.posModel, 0.0).xyz;
    
    return float4(color, 1.0);
}