#include "pch.h"

#include <d3d12.h>
#include <d3dcompiler.h>
#include <d3dx12.h>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "D3DUtil.h"

void GetHardwareAdapter(IDXGIFactory2 *pFactory, IDXGIAdapter1 **ppAdapter)
{
    IDXGIAdapter1 *adapter = nullptr;
    *ppAdapter = nullptr;

    for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // Don't select the Basic Render Driver adapter.
            continue;
        }

        // Check to see if the adapter supports Direct3D 12, but don't create the
        // actual device yet.
        if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
        {
            break;
        }
    }

    *ppAdapter = adapter;
}
void GetSoftwareAdapter(IDXGIFactory2 *pFactory, IDXGIAdapter1 **ppAdapter)
{
    IDXGIAdapter1 *adapter = nullptr;
    *ppAdapter = nullptr;

    for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // Check to see if the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device), nullptr)))
            {
                *ppAdapter = adapter;
                break;
            }
        }
    }
}
void SetDebugLayerInfo(ID3D12Device *pD3DDevice)
{
    ID3D12InfoQueue *pInfoQueue = nullptr;
    pD3DDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
    if (pInfoQueue)
    {
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);

        D3D12_MESSAGE_ID hide[] = {D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE, D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
                                   // Workarounds for debug layer issues on hybrid-graphics systems
                                   D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE,
                                   D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE};
        D3D12_INFO_QUEUE_FILTER filter = {};
        filter.DenyList.NumIDs = (UINT)_countof(hide);
        filter.DenyList.pIDList = hide;
        pInfoQueue->AddStorageFilterEntries(&filter);

        pInfoQueue->Release();
        pInfoQueue = nullptr;
    }
}

void UpdateTexture(ID3D12Device *pD3DDevice, ID3D12GraphicsCommandList *pCommandList, ID3D12Resource *pDestTexResource,
                   ID3D12Resource *pSrcTexResource)
{
    const UINT                         MAX_SUB_RESOURCE_NUM = 32;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT FootPrint[MAX_SUB_RESOURCE_NUM] = {};
    UINT                               Rows[MAX_SUB_RESOURCE_NUM] = {};
    UINT64                             RowSize[MAX_SUB_RESOURCE_NUM] = {};
    UINT64                             TotalBytes = 0;

    D3D12_RESOURCE_DESC Desc = pDestTexResource->GetDesc();
    if (Desc.MipLevels > (UINT)_countof(FootPrint))
        __debugbreak();

    pD3DDevice->GetCopyableFootprints(&Desc, 0, Desc.MipLevels, 0, FootPrint, Rows, RowSize, &TotalBytes);

    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pDestTexResource,
                                                                           D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
                                                                           D3D12_RESOURCE_STATE_COPY_DEST));
    for (UINT i = 0; i < Desc.MipLevels; i++)
    {
        D3D12_TEXTURE_COPY_LOCATION destLocation = {};
        destLocation.PlacedFootprint = FootPrint[i];
        destLocation.pResource = pDestTexResource;
        destLocation.SubresourceIndex = i;
        destLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

        D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
        srcLocation.PlacedFootprint = FootPrint[i];
        srcLocation.pResource = pSrcTexResource;
        srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

        pCommandList->CopyTextureRegion(&destLocation, 0, 0, 0, &srcLocation, nullptr);
    }
    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pDestTexResource,
                                                                           D3D12_RESOURCE_STATE_COPY_DEST,
                                                                           D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE));
}

BYTE *CreateImageFromFile(const wchar_t *filePath, int *pWidth, int *pHeight)
{
    // whar -> char
    size_t convertedChars = 0;
    char   path[MAX_FILE_STRING];

    wcstombs_s(&convertedChars, path, filePath, std::wcslen(filePath) * 4 + 1);
    int         width, height, channels;
    const BYTE *img = stbi_load(path, &width, &height, &channels, 0);

    if (!img)
    {
        //__debugbreak();
        return nullptr;
    }

    BYTE *image = new BYTE[width * height * 4];

    if (channels == 1)
    {
        for (size_t i = 0; i < width * height; i++)
        {
            uint8_t g = img[i * channels + 0];
            for (size_t c = 0; c < 4; c++)
            {
                image[4 * i + c] = g;
            }
        }
    }
    else if (channels == 2)
    {
        for (size_t i = 0; i < width * height; i++)
        {
            for (size_t c = 0; c < 2; c++)
            {
                image[4 * i + c] = img[i * channels + c];
            }
            image[4 * i + 2] = 255;
            image[4 * i + 3] = 255;
        }
    }
    else if (channels == 3)
    {
        for (size_t i = 0; i < width * height; i++)
        {
            for (size_t c = 0; c < 3; c++)
            {
                image[4 * i + c] = img[i * channels + c];
            }
            image[4 * i + 3] = 255;
        }
    }
    else if (channels == 4)
    {
        for (size_t i = 0; i < width * height; i++)
        {
            for (size_t c = 0; c < 4; c++)
            {
                image[4 * i + c] = img[i * channels + c];
            }
        }
    }
    else
    {
        __debugbreak();
    }

    *pWidth = width;
    *pHeight = height;

    delete[] img;

    return image;
}

constexpr void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        __debugbreak();
    }
}

IDxcBlob *CompileShaderLibrary(LPCWSTR fileName, LPCWSTR entryPoint, BOOL disableOptimize)
{
    static IDxcCompiler       *pCompiler = nullptr;
    static IDxcLibrary        *pLibrary = nullptr;
    static IDxcIncludeHandler *dxcIncludeHandler;

    HRESULT hr;

    // Initialize the DXC compiler and compiler helper
    if (!pCompiler)
    {
        ThrowIfFailed(DxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler), (void **)&pCompiler));
        ThrowIfFailed(DxcCreateInstance(CLSID_DxcLibrary, __uuidof(IDxcLibrary), (void **)&pLibrary));
        ThrowIfFailed(pLibrary->CreateIncludeHandler(&dxcIncludeHandler));
    }
    // Open and read the file
    std::ifstream shaderFile(fileName);
    if (shaderFile.good() == false)
    {
        throw std::logic_error("Cannot find shader file");
    }
    std::stringstream strStream;
    strStream << shaderFile.rdbuf();
    std::string sShader = strStream.str();

    // Create blob from the string
    IDxcBlobEncoding *pTextBlob;
    ThrowIfFailed(
        pLibrary->CreateBlobWithEncodingFromPinned((LPBYTE)sShader.c_str(), (uint32_t)sShader.size(), 0, &pTextBlob));

    LPCWSTR pArg[16] = {};
    DWORD   dwArgCount = 0;
    if (disableOptimize)
    {
        pArg[dwArgCount] = L"-Zi";
        dwArgCount++;
        pArg[dwArgCount] = L"-Qembed_debug";
        dwArgCount++;
        pArg[dwArgCount] = L"-Od";
        dwArgCount++;
    }
    else
    {
        pArg[dwArgCount] = L"-O3"; // Optimization level 3
        dwArgCount++;
    }
    // Compile
    IDxcOperationResult *pResult;
    ThrowIfFailed(pCompiler->Compile(pTextBlob, fileName, entryPoint, L"lib_6_5", pArg, dwArgCount, nullptr, 0,
                                     dxcIncludeHandler, &pResult));

    // Verify the result
    HRESULT resultCode;
    ThrowIfFailed(pResult->GetStatus(&resultCode));
    if (FAILED(resultCode))
    {
        IDxcBlobEncoding *pError;
        hr = pResult->GetErrorBuffer(&pError);
        if (FAILED(hr))
        {
            throw std::logic_error("Failed to get shader compiler error");
        }

        // Convert error blob to a string
        std::vector<char> infoLog(pError->GetBufferSize() + 1);
        memcpy(infoLog.data(), pError->GetBufferPointer(), pError->GetBufferSize());
        infoLog[pError->GetBufferSize()] = 0;

        std::string errorMsg = "Shader Compiler Error:\n";
        errorMsg.append(infoLog.data());

        MessageBoxA(nullptr, errorMsg.c_str(), "Error!", MB_OK);
        throw std::logic_error("Failed compile shader");
    }

    IDxcBlob *pBlob;

    ThrowIfFailed(pResult->GetResult(&pBlob));
    return pBlob;
}

IDxcBlob *CompileGraphicsShader(LPCWSTR fileName, LPCWSTR entryPoint, LPCWSTR target, BOOL disableOptimize)
{
    static IDxcCompiler       *pCompiler = nullptr;
    static IDxcLibrary        *pLibrary = nullptr;
    static IDxcIncludeHandler *dxcIncludeHandler;

    HRESULT hr;

    // Initialize the DXC compiler and compiler helper
    if (!pCompiler)
    {
        ThrowIfFailed(DxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler), (void **)&pCompiler));
        ThrowIfFailed(DxcCreateInstance(CLSID_DxcLibrary, __uuidof(IDxcLibrary), (void **)&pLibrary));
        ThrowIfFailed(pLibrary->CreateIncludeHandler(&dxcIncludeHandler));
    }
    // Open and read the file
    std::ifstream shaderFile(fileName);
    if (shaderFile.good() == false)
    {
        throw std::logic_error("Cannot find shader file");
    }
    std::stringstream strStream;
    strStream << shaderFile.rdbuf();
    std::string sShader = strStream.str();

    // Create blob from the string
    IDxcBlobEncoding *pTextBlob;
    ThrowIfFailed(
        pLibrary->CreateBlobWithEncodingFromPinned((LPBYTE)sShader.c_str(), (uint32_t)sShader.size(), 0, &pTextBlob));

    LPCWSTR pArg[16] = {};
    DWORD   dwArgCount = 0;
    if (disableOptimize)
    {
        pArg[dwArgCount] = L"-Zi";
        dwArgCount++;
        pArg[dwArgCount] = L"-Qembed_debug";
        dwArgCount++;
        pArg[dwArgCount] = L"-Od";
        dwArgCount++;
    }
    else
    {
        pArg[dwArgCount] = L"-O3"; // Optimization level 3
        dwArgCount++;
    }
    // Compile
    IDxcOperationResult *pResult;
    ThrowIfFailed(pCompiler->Compile(pTextBlob, fileName, entryPoint, target, pArg, dwArgCount, nullptr, 0,
                                     dxcIncludeHandler, &pResult));

    // Verify the result
    HRESULT resultCode;
    ThrowIfFailed(pResult->GetStatus(&resultCode));
    if (FAILED(resultCode))
    {
        IDxcBlobEncoding *pError;
        hr = pResult->GetErrorBuffer(&pError);
        if (FAILED(hr))
        {
            throw std::logic_error("Failed to get shader compiler error");
        }

        // Convert error blob to a string
        std::vector<char> infoLog(pError->GetBufferSize() + 1);
        memcpy(infoLog.data(), pError->GetBufferPointer(), pError->GetBufferSize());
        infoLog[pError->GetBufferSize()] = 0;

        std::string errorMsg = "Shader Compiler Error:\n";
        errorMsg.append(infoLog.data());

        MessageBoxA(nullptr, errorMsg.c_str(), "Error!", MB_OK);
        throw std::logic_error("Failed compile shader");
    }

    IDxcBlob *pBlob;

    ThrowIfFailed(pResult->GetResult(&pBlob));
    return pBlob;
}

IDxcBlob *CompileShaderLibrary(LPCWSTR fileName, BOOL disableOptimize)
{
    return CompileShaderLibrary(fileName, L"", disableOptimize);
}
