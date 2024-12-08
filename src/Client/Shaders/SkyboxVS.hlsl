#include "Common.hlsli"

struct SkyboxVertexShaderInput
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD0;
};

struct SkyboxPixelShaderInput
{
    float4 posProj : SV_POSITION;
    float3 posModel : POSITION;
};

SkyboxPixelShaderInput main(SkyboxVertexShaderInput input)
{
    SkyboxPixelShaderInput output;
    
    output.posModel = input.position;
    output.posProj = mul(float4(input.position, 0.0), view);
    output.posProj = mul(float4(output.posProj.xyz, 1.0), proj);
    //output.posProj = mul(float4(input.position, 1.0), viewProj);
    
    return output;
}