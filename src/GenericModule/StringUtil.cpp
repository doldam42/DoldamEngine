#include "pch.h"

#include "StringUtil.h"
#include <Windows.h>
#include <filesystem>

void s2ws(const char *inStr, wchar_t *outWStr)
{
    int len = (int)strlen(inStr);
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, inStr, len, NULL, 0);
    MultiByteToWideChar(CP_UTF8, 0, inStr, len, outWStr, size_needed + 1);
}

void ws2s(const wchar_t *inWStr, char *outStr)
{
    int len = (int)wcslen(inWStr);
    int size_needed = WideCharToMultiByte(CP_ACP, 0, inWStr, len, NULL, 0, NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, inWStr, len, outStr, size_needed + 1, NULL, NULL);
}

bool TryGetExtension(const wchar_t *filename, wchar_t *outExtension)
{
    const std::wstring ext = std::filesystem::path(filename).extension();
    for (size_t index = 0; index < ext.size(); index++)
    {
        outExtension[index] = towlower(ext[index]);
    }

    return !ext.empty();
}

bool IsFile(const wchar_t *filename) \
{ 
    std::filesystem::path p(filename);
    return std::filesystem::is_regular_file(p) && p.has_filename();
}
