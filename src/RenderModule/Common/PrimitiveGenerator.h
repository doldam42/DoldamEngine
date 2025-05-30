#pragma once

#include <string>
#include <vector>

#include "../MathModule/MathHeaders.h"

struct BASIC_MESH
{
    ID3D12Resource *pVertexBuffer = nullptr;
    ID3D12Resource *pIndexBuffer = nullptr;

    D3D12_INDEX_BUFFER_VIEW  IndexBufferView = {};
    D3D12_VERTEX_BUFFER_VIEW VertexBufferView = {};

    UINT numIndices = 0;
};

class PrimitiveGenerator
{
  public:
    static const BASIC_MESH &GetQuadMesh();

    static IRenderMesh *MakeSquare(const float scale = 1.0f);
    static IRenderMesh *MakeSquareGrid(const int numSlices, const int numStacks, const float scale = 1.0f);
    static IRenderMesh *MakeBox(const float scale = 1.0f);
    static IRenderMesh *MakeWireBox(const Vector3 center, const Vector3 extends);
    static IRenderMesh *MakeCylinder(const float bottomRadius, const float topRadius, float height, int numSlices);
    static IRenderMesh *MakeSphere(const float radius, const int numSlices, const int numStacks);
    static IRenderMesh *MakeTetrahedron();
    static IRenderMesh *MakeIcosahedron();
    static IRenderMesh *SubdivideToSphere(const float radius);
};