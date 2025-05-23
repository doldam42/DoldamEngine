#pragma once

struct SimpleVertex
{
    Vector3 position;
    Vector2 texcoord;
};

struct BasicVertex
{
    Vector3 position;
    // Vector4 color;
    Vector3 normal;
    Vector2 texcoord;
    Vector3 tangent;
};

struct SkinnedVertex : public BasicVertex
{
    float   blendWeights[4] = {0.0f, 0.0f, 0.0f, 0.0f}; // BLENDWEIGHT0 and 1
    uint8_t boneIndices[4] = {0, 0, 0, 0};              // BLENDINDICES0 and 1
};
