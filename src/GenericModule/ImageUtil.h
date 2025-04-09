#pragma once

BOOL CreateHeightMapFromFile(const WCHAR *path, UINT8** ppImage, int* pWidth, int* pHeight);
BOOL CreateHeightMapFromFile(const WCHAR *path, UINT16** ppImage, int* pWidth, int* pHeight);
