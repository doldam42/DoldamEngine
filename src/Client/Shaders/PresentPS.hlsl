Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

struct PSInput
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
    const float gamma = 2.2;
    const float exposure = 1;
    
    float3 hdrColor = g_texture.Sample(g_sampler, input.texcoord).xyz;
    
    float3 mapped = float3(1, 1, 1) - exp(-hdrColor * exposure);
    
    float3 invGamma = float3(1, 1, 1) / gamma;
    mapped = pow(mapped, invGamma);
    
    return float4(mapped, 1);
}
