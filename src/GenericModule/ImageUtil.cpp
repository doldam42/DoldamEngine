#include "pch.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "ImageUtil.h"

BOOL CreateHeightMapFromFile(const WCHAR *path, UINT8 **ppImage, int *pWidth, int *pHeight) 
{
    char filename[MAX_PATH] = {'\0'};
    ws2s(path, filename);
    int      width, height, channels;
    UINT8 *pImage = stbi_load(filename, &width, &height, &channels, 0);
    if (!pImage)
    {
        DASSERT(false);
        return FALSE;
    }

    if (channels == 1)
    {
        *ppImage = pImage;
        *pWidth = width;
        *pHeight = height;
        return TRUE;
    }
    
    UINT8 *pNew = new UINT8[width * height];
    ZeroMemory(pNew, sizeof(UINT8) * width * height);
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            int  idx = j * width + i;
            pNew[idx] = pImage[idx * channels];
        }
    }

    *ppImage = pNew;
    *pWidth = width;
    *pHeight = height;
    delete[] pImage;

    return TRUE;
}

BOOL CreateHeightMapFromFile(const WCHAR *path, UINT16 **ppImage, int *pWidth, int *pHeight)
{
    char filename[MAX_PATH] = {'\0'};
    ws2s(path, filename);
    int   width, height, channels;
    UINT16 *pImage = stbi_load_16(filename, &width, &height, &channels, 0);
    if (!pImage)
    {
        assert(false);
        return FALSE;
    }

    if (channels == 1)
    {
        *ppImage = pImage;
        *pWidth = width;
        *pHeight = height;
        return TRUE;
    }

    UINT16 *pNew = new UINT16[width * height];
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            int idx = j * width + i;
            pNew[idx] = pImage[idx * channels];
        }
    }
    *ppImage = pNew;
    *pWidth = width;
    *pHeight = height;
    delete[] pImage;
    return TRUE;
}
