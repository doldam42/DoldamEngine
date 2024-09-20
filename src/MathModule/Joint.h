#pragma once

struct Joint
{
    wchar_t  name[MAX_NAME] = { 0 };
    int    parentIndex = -1;
    Matrix globalBindposeInverse = Matrix::Identity;
};
