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
    size_t len = wcslen(filename);
    
    int idx = 0;
    for (int i = 0; i < len; i++)
    {
        if (filename[i] == L'.')
        {
            idx = i;
        }
    }

    int outLen = 0;
    for (int i = idx; i < len; i++)
    {
        outExtension[outLen] = towlower(filename[i]);
        outLen++;
    }

    return outLen != 0;
}

bool ChangeExtension(const WCHAR *ext, WCHAR *filename) 
{ 
    size_t len = wcslen(filename);
    size_t extLen = wcslen(ext);

    int idx = 0;
    for (int i = 0; i < len; i++)
    {
        if (filename[i] == L'.')
        {
            idx = i;
        }
    }

    int outLen = 0;
    for (int i = idx; i < len; i++)
    {
        if (outLen < extLen)
        {
            filename[i] = ext[outLen];
        }
        else
        {
            filename[i] = '\0';
        }
        outLen++;
    }

    return outLen != 0;
}

bool IsFile(const wchar_t *filename)
{ 
    std::filesystem::path p(filename);
    return std::filesystem::is_regular_file(p) && p.has_filename();
}
