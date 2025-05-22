#pragma once

struct Keyframe
{
    WCHAR  BindingJointName[MAX_NAME] = {0};
    UINT   NumKeys = 0;
    Matrix pKeys[0];

    inline size_t GetSize() const { return sizeof(Keyframe) + sizeof(Matrix) * NumKeys; }
};

Keyframe *CreateKeyframe(const WCHAR *pJointName, const Matrix *keys, UINT numKeys);

void DeleteKeyframe(Keyframe *pKeyframe);