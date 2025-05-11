#include "AOIT.hlsli"
#include "FragmentList.hlsli"

struct PSInput
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
	uint i;
    uint nodeOffset;
    uint2 screenAddress = int2(input.position.xy);

    // Get offset to the first node
    uint firstNodeOffset = FL_GetFirstNodeOffset(screenAddress);

    AOITData data;
    // Initialize AVSM data
    [unroll] for (i = 0; i < OIT_RT_COUNT; ++i)
    {
        data.depth[i] = IOT_EMPTY_NODE_DEPTH.xxxx;
        data.trans[i] = OIT_FIRT_NODE_TRANS.xxxx;
    }

    // Fetch all nodes and add them to our visibiity function
    nodeOffset = firstNodeOffset;
    [loop] while (nodeOffset != FL_NODE_LIST_NULL)
    {
        // Get node..
        FragmentListNode node = FL_GetNode(nodeOffset);

        float depth;
        uint  coverageMask;
        FL_UnpackDepthAndCoverage(node.depth, depth, coverageMask);

        // Unpack color
        float4 nodeColor = FL_UnpackColor(node.color);
        AOITInsertFragment(depth, saturate(1.0 - nodeColor.w), data);

        // Move to next node
        nodeOffset = node.next;
    }
    float3 color = float3(0, 0, 0);
    // Fetch all nodes again and composite them
    nodeOffset = firstNodeOffset;
    [loop] while (nodeOffset != FL_NODE_LIST_NULL)
    {
        // Get node..
        FragmentListNode node = FL_GetNode(nodeOffset);
        float            depth;
        uint             coverageMask;
        FL_UnpackDepthAndCoverage(node.depth, depth, coverageMask);

        // Unpack color
        float4       nodeColor = FL_UnpackColor(node.color);
        AOITFragment frag = AOITFindFragment(data, depth);
        float        vis = frag.index == 0 ? 1.0f : frag.transA;
        color += nodeColor.xyz * nodeColor.www * vis.xxx;

        // Move to next node
        nodeOffset = node.next;
    }

    float4 blendColor = float4(color, data.trans[OIT_RT_COUNT - 1][3]);
    return blendColor;
}