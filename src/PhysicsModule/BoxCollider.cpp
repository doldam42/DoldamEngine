#include "pch.h"
#include "BoxCollider.h"

void BoxCollider::GetCorners(Vector3 pOutCorners[8])
{
    Bounds b(-HalfExtent, HalfExtent);

    b.GetCorners(pOutCorners);
    for (int i = 0; i < CORNER_COUNT; i++)
    {
        pOutCorners[i] = Vector3::Transform(pOutCorners[i], Rotation) + Position;
    }
}
