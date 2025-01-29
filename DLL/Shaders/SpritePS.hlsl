Texture2D texDiffuse : register(t0);
SamplerState samplerDiffuse : register(s0);

struct PSInput
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
    float4 texColor = texDiffuse.Sample(samplerDiffuse, input.texcoord);
    return texColor * input.color;
}
