#include "pch.h"
#include "Keyframe.h"

Keyframe *CreateKeyframe(const WCHAR *pJointName, const Matrix *keys, UINT numKeys)
{
    int requiredSize = sizeof(Keyframe) + sizeof(Matrix) * numKeys;

    Keyframe *pData = (Keyframe *)malloc(requiredSize);
    pData->NumKeys = numKeys;
    wcscpy_s(pData->BindingJointName, pJointName);
    memcpy(pData->pKeys, keys, sizeof(Matrix) * numKeys);
    return pData;
}

void DeleteKeyframe(Keyframe *pKeyframe)
{
    if (pKeyframe)
    {
        free(pKeyframe);
    }
}
