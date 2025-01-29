
struct PSInput
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD0;
};

struct VSInput
{
    float4 pos : POSITION;
    float2 texcoord : TEXCOORD0;
};

cbuffer SpriteConstants : register(b0)
{
    float2 g_screenRes;
    float2 g_pos;
    float2 g_scale;
    float2 g_texSize;
    float2 g_texSamplePos;
    float2 g_texSampleSize;
    float g_z;
    float g_alpha;
    float g_spriteDummy[50];
};

PSInput main(VSInput input)
{
    PSInput output;
    
    float2 scale = (g_texSize / g_screenRes) * g_scale;
    float2 offset = (g_pos / g_screenRes);
    float2 pos = input.pos.xy * scale + offset;
    output.position = float4(pos.xy * float2(2, -2) + float2(-1, 1), g_z, 1);
    
    float2 texScale = (g_texSampleSize / g_texSize);
    float2 texOffset = (g_texSamplePos / g_texSize);
    output.texcoord = input.texcoord * texScale + texOffset;
    
    output.color = float4(1, 1, 1, 1);
    return output;
}