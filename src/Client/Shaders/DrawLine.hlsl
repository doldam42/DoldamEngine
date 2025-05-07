#define MAX_LIGHTS 3
struct Light
{
    float3 radiance; // strength
    float  fallOffStart;
    float3 direction;
    float  fallOffEnd;
    float3 position;
    float  spotPower;

    uint  type;
    float radius; // ¹ÝÁö¸§

    float haloRadius;
    float haloStrength;

    float4x4 viewProj;
    float4x4 invProj;
};

cbuffer g_cb : register(b0)
{
    float4x4 view;
    float4x4 invView;
    float4x4 proj;
    float4x4 invProj;
    float4x4 viewProj;
    float4x4 invViewProj; // Proj -> World
    float4x4 projectionViewProj;

    float3 eyeWorld;
    float  strengthIBL;

    Light lights[MAX_LIGHTS];
    uint  useTextureProjection;
    float gcDummy2[59];
}

struct PSInput
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

PSInput VSMain(float3 pos : POSITION, float4 color : COLOR)
{
    PSInput output;
    
    output.position = mul(float4(pos, 1.0f), viewProj);
    output.color = color;
    return output;
}

float4 PSMain(PSInput input) : SV_Target { return input.color; }