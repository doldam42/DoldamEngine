
struct PresentVertexShaderInput
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD0;
};

struct PresentPixelShaderInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

PresentPixelShaderInput main(PresentVertexShaderInput input)
{
    PresentPixelShaderInput output;
    
    output.position = float4(input.position.xy * float2(2, -2) + float2(-1, 1), 0, 1);
    // output.position = float4(input.position,1);
    output.texcoord = input.texcoord;

    return output;
}
