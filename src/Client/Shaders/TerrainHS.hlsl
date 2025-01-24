cbuffer TerrainConstantBuffer : register(b1)
{
    uint  numSlice;
    uint  numStack;
    float scale;
    float heightScale;
    float tessFactor;
    uint  tcDummy[59];
}

struct HSInput
{
    float3 posWorld : POSITION;
    float2 texcoord : TEXCOORD;
};
struct DSInput
{
    float3 posWorld : POSITION;
    float2 texcoord : TEXCOORD;
};
struct PatchConstOutput
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

PatchConstOutput MyPatchConstantFunc(InputPatch<HSInput, 4> patch, uint patchID : SV_PrimitiveID)
{
    PatchConstOutput pt;
    
    pt.edges[0] = tessFactor;
    pt.edges[1] = tessFactor;
    pt.edges[2] = tessFactor;
    pt.edges[3] = tessFactor;
    pt.inside[0] = tessFactor;
    pt.inside[1] = tessFactor;

    return pt;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("MyPatchConstantFunc")]
[maxtessfactor(64.0f)] 
DSInput HSMain(InputPatch<HSInput, 4> p, uint uCPID : SV_OutputControlPointID)
{
    DSInput output;

    output.posWorld = p[uCPID].posWorld;
    output.texcoord = p[uCPID].texcoord;

    return output;
}