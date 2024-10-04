#include "pch.h"
#include "GameUtils.h"

void GameUtils::s2ws(const char *inStr, wchar_t *outWStr)
{
    int len = (int)strlen(inStr);
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, inStr, len, NULL, 0);
    MultiByteToWideChar(CP_UTF8, 0, inStr, len, outWStr, size_needed + 1);
}

void GameUtils::ws2s(const wchar_t *inWStr, char *outStr)
{
    int len = (int)wcslen(inWStr);
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, inWStr, len, NULL, 0, NULL, NULL);
    WideCharToMultiByte(CP_UTF8, 0, inWStr, len, outStr, size_needed + 1, NULL, NULL);
}

void GameUtils::argsort(const float *arr, int *outArr, int numElement)
{
    std::sort(outArr, outArr + numElement, [&arr](size_t i1, size_t i2) { return arr[i1] > arr[i2]; });
}