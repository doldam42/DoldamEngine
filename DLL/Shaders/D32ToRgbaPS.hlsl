Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

struct PSInput
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
    float farZ = 100.0;
    //float nearZ = 0.1;
    
    float4 texColor = g_texture.Sample(g_sampler, input.texcoord);
    float r = texColor.r / texColor.w;
    float depthValue = (1 - r);
    
    return float4(depthValue, depthValue, depthValue, 1);
}