#pragma once

const UINT MAX_FILE_STRING = 120;

void GetHardwareAdapter(IDXGIFactory2 *pFactory, IDXGIAdapter1 **ppAdapter);
void GetSoftwareAdapter(IDXGIFactory2 *pFactory, IDXGIAdapter1 **ppAdapter);
void SetDebugLayerInfo(ID3D12Device *pD3DDevice);

void UpdateTexture(ID3D12Device *pD3DDevice, ID3D12GraphicsCommandList *pCommandList, ID3D12Resource *pDestTexResource,
                   ID3D12Resource *pSrcTexResource);

const BYTE *CreateImageFromFile(const wchar_t *filePath, int *pWidth,
                                int *pHeight); // 4채널 이미지를 반환하는 함수

// Reference: NVIDIA RAYTRACING SAMPLE CODE
IDxcBlob *CompileShaderLibrary(LPCWSTR fileName, BOOL disableOptimize);

IDxcBlob *CompileShaderLibrary(LPCWSTR fileName, LPCWSTR entryPoint, BOOL disableOptimize);