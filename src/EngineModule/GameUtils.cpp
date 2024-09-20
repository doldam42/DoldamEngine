#include "pch.h"
#include "GameUtils.h"

void GameUtils::s2ws(const char *inStr, wchar_t *outWStr)
{
    int len = (int)strlen(inStr);
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, inStr, len, NULL, 0);
    MultiByteToWideChar(CP_UTF8, 0, inStr, len, outWStr, size_needed);
}

void GameUtils::ws2s(const wchar_t *inWStr, char *outStr)
{
    int len = (int)wcslen(inWStr);
    int         size_needed = WideCharToMultiByte(CP_UTF8, 0, inWStr, len, NULL, 0, NULL, NULL);
    WideCharToMultiByte(CP_UTF8, 0, inWStr, len, outStr, size_needed, NULL, NULL);
}

void GameUtils::argsort(const float *arr, int *outArr, int numElement)
{
    std::sort(outArr, outArr + numElement, [&arr](size_t i1, size_t i2) { return arr[i1] > arr[i2]; });
}

// elf Hash
size_t GameUtils::CreateHash(const wchar_t *inWStr, uint32_t wstrLen)
{
    long hashValue = 0;
    for (int Pos = 0; Pos < wstrLen; Pos++)
    {                                                      // use all elements
        hashValue = (hashValue << 4) + inWStr[Pos]; // shift/mix
        long hiBits = hashValue & 0xF000000000000000;      // get high nybble
        if (hiBits != 0)
            hashValue ^= hiBits >> 56; // xor high nybble with second nybble
        hashValue &= ~hiBits;          // clear high nybble
    }
    return hashValue;
}
