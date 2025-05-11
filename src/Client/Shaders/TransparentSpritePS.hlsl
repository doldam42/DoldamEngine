#include "FragmentList.hlsli"

Texture2D texDiffuse : register(t0);
SamplerState samplerDiffuse : register(s0);

struct PSInput
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Capture fragments phase
//--------------------------------------------------------------------------------------
[earlydepthstencil]
void main(PSInput input, in uint coverageMask : SV_Coverage)
{
    float4 texColor = texDiffuse.Sample(samplerDiffuse, input.texcoord);
    
    float4 result = texColor * input.color;
    
    float surfaceDepth = input.position.z;

    // can't use discard here because it crashes ATI shader compiler
    if (0 != result.a)
    {
        // Get fragment viewport coordinates
        uint newNodeAddress;
        int2 screenAddress = int2(input.position.xy);

        if (FL_AllocNode(newNodeAddress))
        {
            // Fill node
            FragmentListNode node;
            node.color = FL_PackColor(result);
            node.depth = FL_PackDepthAndCoverage(surfaceDepth, coverageMask);

            // Insert node!
            FL_InsertNode(screenAddress, newNodeAddress, node);
        }
    }
}
