#pragma once

#include "stdint.h"

class GameUtils
{
  public:
    static void   s2ws(const char *inStr, wchar_t *outWStr);
    static void   ws2s(const wchar_t *inWStr, char *outStr);
    // static std::wstring s2ws(const std::string &str);
    // static std::string  ws2s(const std::wstring &wstr);
    static void   argsort(const float *arr, int *outArr, int numElement);
    static size_t CreateHash(const wchar_t *inWStr, uint32_t wstrLen);
};
