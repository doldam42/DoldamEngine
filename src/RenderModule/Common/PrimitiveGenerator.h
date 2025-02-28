#pragma once

#include <string>
#include <vector>

#include "../MathModule/MathHeaders.h"

class PrimitiveGenerator
{
  public:
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