#include "pch.h"
#include "Box.h"

void Box::LocalToWorld(Box *pOutBox, const Matrix &worldTM) const
{
    using namespace DirectX;
    FXMMATRIX M = worldTM;

    Vector3 Center;
    Vector3 Extents;
    GetCenterAndExtent(&Center, &Extents);

    // Load center and extents.
    XMVECTOR vCenter = XMLoadFloat3(&Center);
    XMVECTOR vExtents = XMLoadFloat3(&Extents);

    // Compute and transform the corners and find new min/max bounds.
    XMVECTOR Corner = XMVectorMultiplyAdd(vExtents, g_BoxOffset[0], vCenter);
    Corner = XMVector3Transform(Corner, M);

    XMVECTOR Min, Max;
    Min = Max = Corner;

    for (size_t i = 1; i < CORNER_COUNT; ++i)
    {
        Corner = XMVectorMultiplyAdd(vExtents, g_BoxOffset[i], vCenter);
        Corner = XMVector3Transform(Corner, M);

        Min = XMVectorMin(Min, Corner);
        Max = XMVectorMax(Max, Corner);
    }

    pOutBox->Min = Min;
    pOutBox->Max = Max;
}