#pragma once

#include <string>
#include <vector>

#include "../MathModule/MathHeaders.h"

class Model;
class AnimationClip;
class GeometryGenerator
{
  public:
    static Model *ReadFromFile(const wchar_t *basePath, const wchar_t *filename);

    static AnimationClip *ReadAnimationFromFile(const wchar_t *basePath, const wchar_t *filename);

    static void Normalize(const Vector3 center, const float longestLength, Model *pInOutModel);

    static Model *MakeSquare(const float scale = 1.0f);
    static Model *MakeSquareGrid(const int numSlices, const int numStacks, const float scale = 1.0f);
    static Model *MakeBox(const float scale = 1.0f);
    static Model *MakeWireBox(const Vector3 center, const Vector3 extends);
    static Model *MakeSphere(const float radius, const int numSlices, const int numStacks);

    static Model *MakeDiamond(const float scale = 1.0f);

    static Model *MakeCylinder(const float bottomRadius, const float topRadius, float height, int numSlices);
    
    static Model *MakeTetrahedron();
    static Model *MakeIcosahedron();
    static Model *SubdivideToSphere(const float radius, Model Model);
};