Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

struct PSInput
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
    float depthValue = g_texture.Sample(g_sampler, input.texcoord).r;
    
    return float4(depthValue, depthValue, depthValue, 1);
}