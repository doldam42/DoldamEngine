#pragma once

void s2ws(const char *inStr, wchar_t *outWStr);
void ws2s(const wchar_t *inWStr, char *outStr);

bool TryGetExtension(const wchar_t *filename, wchar_t *outExtension);

bool ChangeExtension(const WCHAR *ext, WCHAR *filename);

bool IsFile(const wchar_t *filename);