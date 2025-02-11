Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

struct PSInput
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
};

float3 FilmicToneMapping(float3 color)
{
    color = max(float3(0, 0, 0), color);
    color = (color * (6.2 * color + 0.5)) / (color * (6.2 * color + 1.7) + 0.06);
    return color;
}

float3 LinearToneMapping(float3 color, float gamma, float exposure)
{
    float3 invGamma = float3(1, 1, 1) / gamma;

    color = clamp(exposure * color, 0., 1.);
    color = pow(color, invGamma);
    return color;
}

float3 Uncharted2ToneMapping(float3 color, float gamma, float exposure)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    float W = 11.2;

    color *= exposure;
    color = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
    float white = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
    color /= white;
    color = pow(color, float3(1.0, 1.0, 1.0) / gamma);
    return color;
}

float3 lumaBasedReinhardToneMapping(float3 color, float gamma, float exposure)
{
    float3 invGamma = float3(1, 1, 1) / gamma;
    float  luma = dot(color, float3(0.2126, 0.7152, 0.0722));
    float  toneMappedLuma = luma / (1. + luma);
    color *= toneMappedLuma / luma;
    color = pow(color, invGamma);
    return color;
}

float4 main(PSInput input) : SV_TARGET
{
    const float gamma = 2.2;
    const float exposure = 1;

    float3 hdrColor = g_texture.Sample(g_sampler, input.texcoord).rgb;
    
    // Tone Mapping
    //hdrColor = FilmicToneMapping(hdrColor);
    hdrColor = LinearToneMapping(hdrColor, gamma, exposure);

    return float4(hdrColor, 1);
}
