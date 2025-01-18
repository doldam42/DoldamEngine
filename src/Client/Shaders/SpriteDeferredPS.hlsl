Texture2D    texDiffuse : register(t0);
SamplerState samplerDiffuse : register(s0);

struct PS_OUTPUT
{
    float4 albedo : SV_Target0;
    float4 normal : SV_Target1;
    /*
     * r : reflect factor
     * g : roughness factor
     * b : metallic factor
     * a : transparancy factor
     */
    float4 factors : SV_Target2;
};

struct PSInput
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD0;
};

PS_OUTPUT main(PSInput input)
{
    PS_OUTPUT output;

    float4 texColor = texDiffuse.Sample(samplerDiffuse, input.texcoord);
    output.albedo = texColor * input.color;
    output.normal = 0.0;
    output.factors = 0.0;

    return output;
}
