#include "pch.h"

#include "../GenericModule/HashTable.h"
#include "../GenericModule/LinkedList.h"

#include "D3D12Renderer.h"
#include "D3D12ResourceManager.h"

#include "TextureManager.h"

TEXTURE_HANDLE *TextureManager::AllocTextureHandle()
{
    TEXTURE_HANDLE *pTexHandle = new TEXTURE_HANDLE;
    memset(pTexHandle, 0, sizeof(TEXTURE_HANDLE));
    pTexHandle->link.pItem = pTexHandle;
    LinkToLinkedListFIFO(&m_pTexLinkHead, &m_pTexLinkTail, &pTexHandle->link);
    pTexHandle->refCount = 1;
    return pTexHandle;
}

UINT TextureManager::DeallocTextureHandle(TEXTURE_HANDLE *pTexHandle)
{
    ID3D12Device *pD3DDevice = m_pRenderer->INL_GetD3DDevice();

    if (!pTexHandle->refCount)
        __debugbreak();

    UINT refCount = --pTexHandle->refCount;
    if (!refCount)
    {
        if (pTexHandle->pTexture)
        {
            pTexHandle->pTexture->Release();
            pTexHandle->pTexture = nullptr;
        }
        if (pTexHandle->pUploadBuffer)
        {
            pTexHandle->pUploadBuffer->Release();
            pTexHandle->pUploadBuffer = nullptr;
        }
        if (pTexHandle->srv.cpuHandle.ptr)
        {
            m_pResourceManager->DeallocDescriptorTable(&pTexHandle->srv);
            pTexHandle->srv = {};
        }
        if (pTexHandle->pSearchHandle)
        {
            m_pHashTable->Delete(pTexHandle->pSearchHandle);
            pTexHandle->pSearchHandle = nullptr;
        }
        UnLinkFromLinkedList(&m_pTexLinkHead, &m_pTexLinkTail, &pTexHandle->link);

        delete pTexHandle;
    }
    return refCount;
}

void TextureManager::Cleanup()
{
    if (m_pTexLinkHead)
    {
        // texture resource leak!!!
        __debugbreak();
    }
    if (m_pHashTable)
    {
        delete m_pHashTable;
        m_pHashTable = nullptr;
    }
}

BOOL TextureManager::Initialize(D3D12Renderer *pRenderer, UINT maxBucketNum, UINT maxFileNum)
{
    m_pRenderer = pRenderer;
    m_pResourceManager = pRenderer->INL_GetResourceManager();

    m_pHashTable = new HashTable();
    m_pHashTable->Initialize(maxBucketNum, MAX_PATH * sizeof(WCHAR), maxFileNum);

    return TRUE;
}

TEXTURE_HANDLE *TextureManager::CreateTextureFromFile(const WCHAR *filename, BOOL isCubemap)
{
    ID3D12Device *pD3DDevice = m_pRenderer->INL_GetD3DDevice();

    ID3D12Resource     *pTexResource = nullptr;
    DESCRIPTOR_HANDLE   srv = {};
    D3D12_RESOURCE_DESC desc = {};
    TEXTURE_HANDLE     *pTexHandle = nullptr;

    wchar_t ext[8] = {0};

    UINT fileNameLen = (DWORD)wcslen(filename);
    UINT keySize = fileNameLen * sizeof(WCHAR);

    if (m_pHashTable->Select((void **)&pTexHandle, 1, filename, keySize))
    {
        pTexHandle->refCount++;
    }
    else
    {
        if (wcslen(filename) == 0 || !TryGetExtension(filename, ext))
            return nullptr;

        BOOL result = !wcscmp(L".dds", ext) ? m_pResourceManager->CreateTextureFromDDS(&pTexResource, &desc, filename)
                                            : m_pResourceManager->CreateTextureFromWIC(&pTexResource, &desc, filename);
        if (result)
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
            SRVDesc.Format = desc.Format;
            SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            SRVDesc.ViewDimension = isCubemap ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURE2D;
            SRVDesc.Texture2D.MipLevels = desc.MipLevels;

            if (m_pResourceManager->AllocDescriptorTable(&srv, 1))
            {
                pD3DDevice->CreateShaderResourceView(pTexResource, &SRVDesc, srv.cpuHandle);

                pTexHandle = AllocTextureHandle();
                pTexHandle->pTexture = pTexResource;
                pTexHandle->IsFromFile = TRUE;
                pTexHandle->srv = srv;

                pTexHandle->pSearchHandle = m_pHashTable->Insert((void *)pTexHandle, filename, keySize);
                if (!pTexHandle->pSearchHandle)
                    __debugbreak();
            }
            else
            {
                pTexResource->Release();
                pTexResource = nullptr;
            }
        }
    }
    return pTexHandle;
}

TEXTURE_HANDLE *TextureManager::CreateDynamicTexture(UINT texWidth, UINT texHeight)
{
    ID3D12Device   *pD3DDevice = m_pRenderer->INL_GetD3DDevice();
    TEXTURE_HANDLE *pTexHandle = nullptr;

    ID3D12Resource   *pTexResource = nullptr;
    ID3D12Resource   *pUploadBuffer = nullptr;
    DESCRIPTOR_HANDLE srv = {};

    DXGI_FORMAT TexFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    if (m_pResourceManager->CreateTexturePair(&pTexResource, &pUploadBuffer, texWidth, texHeight, TexFormat))
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
        SRVDesc.Format = TexFormat;
        SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        SRVDesc.Texture2D.MipLevels = 1;

        if (m_pResourceManager->AllocDescriptorTable(&srv, 1))
        {
            pD3DDevice->CreateShaderResourceView(pTexResource, &SRVDesc, srv.cpuHandle);

            pTexHandle = AllocTextureHandle();
            pTexHandle->pTexture = pTexResource;
            pTexHandle->pUploadBuffer = pUploadBuffer;
            pTexHandle->srv = srv;
        }
        else
        {
            pTexResource->Release();
            pTexResource = nullptr;

            pUploadBuffer->Release();
            pUploadBuffer = nullptr;
        }
    }

    return pTexHandle;
}

TEXTURE_HANDLE *TextureManager::CreateImmutableTexture(UINT texWidth, UINT texHeight, DXGI_FORMAT format,
                                                       const BYTE *pInitImage)
{
    ID3D12Device   *pD3DDevice = m_pRenderer->INL_GetD3DDevice();
    TEXTURE_HANDLE *pTexHandle = nullptr;

    ID3D12Resource   *pTexResource = nullptr;
    DESCRIPTOR_HANDLE srv = {};

    if (SUCCEEDED(m_pResourceManager->CreateTextureFromMemory(&pTexResource, texWidth, texHeight, format, pInitImage)))
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
        SRVDesc.Format = format;
        SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        SRVDesc.Texture2D.MipLevels = 1;

        if (m_pResourceManager->AllocDescriptorTable(&srv, 1))
        {
            pD3DDevice->CreateShaderResourceView(pTexResource, &SRVDesc, srv.cpuHandle);

            pTexHandle = AllocTextureHandle();
            pTexHandle->pTexture = pTexResource;
            pTexHandle->srv = srv;
        }
        else
        {
            pTexResource->Release();
            pTexResource = nullptr;
        }
    }

    return pTexHandle;
}

TEXTURE_HANDLE *TextureManager::CreateMetallicRoughnessTexture(const WCHAR *metallicFilename,
                                                               const WCHAR *roughneessFilename)
{
    ID3D12Device *pD3DDevice = m_pRenderer->INL_GetD3DDevice();

    ID3D12Resource   *pTexResource = nullptr;
    DESCRIPTOR_HANDLE srv = {};
    TEXTURE_HANDLE   *pTexHandle = nullptr;

    // metallicFilename으로 texture 저장
    UINT fileNameLen = (DWORD)wcslen(metallicFilename);
    UINT keySize = fileNameLen * sizeof(WCHAR);

    if (m_pHashTable->Select((void **)&pTexHandle, 1, metallicFilename, keySize))
    {
        pTexHandle->refCount++;
    }
    else
    {
        int        mWidth = 0, mHeight = 0;
        int        rWidth = 0, rHeight = 0;
        const BYTE *mImage = nullptr;
        const BYTE *rImage = nullptr;

        BYTE *combinedImage = nullptr;
        BOOL  result = FALSE;
        
        if (!wcscmp(metallicFilename, roughneessFilename))
        {
            if (wcsstr(metallicFilename, L".dds") != NULL || wcsstr(metallicFilename, L".DDS"))
            {
                if (FAILED(m_pResourceManager->CreateDDSTextureFromFile(&pTexResource, metallicFilename, false)))
                    __debugbreak();
                result = TRUE;
            }
            else
            {
                mImage = CreateImageFromFile(metallicFilename, &mWidth, &mHeight);

                if (mImage)
                {
                    combinedImage = new BYTE[mWidth * mHeight * 4];
                    for (size_t i = 0; i < size_t(mWidth * mHeight); i++)
                    {
                        combinedImage[4 * i + 1] = mImage[4 * i]; // Green = Roughness
                        combinedImage[4 * i + 2] = mImage[4 * i]; // Blue = Metalness
                    }
                }
            }
        }
        else
        {
            if (wcslen(metallicFilename) != 0)
            {
                mImage = CreateImageFromFile(metallicFilename, &mWidth, &mHeight);
            }
            if (wcslen(roughneessFilename) != 0)
            {
                rImage = CreateImageFromFile(roughneessFilename, &rWidth, &rHeight);
            }

            if (mWidth != rWidth || mHeight != rHeight || !mImage || !rImage)
            {
                __debugbreak();
                goto lb_return;
            }

            // 4채널
            combinedImage = new BYTE[mWidth * mHeight * 4];
            memset(combinedImage, 0, sizeof(BYTE) * mWidth * mHeight * 4);

            for (size_t i = 0; i < size_t(mWidth * mHeight); i++)
            {
                combinedImage[4 * i + 1] = rImage[4 * i]; // Green = Roughness
                combinedImage[4 * i + 2] = mImage[4 * i]; // Blue = Metalness
            }
            result = m_pResourceManager->CreateTextureFromMemory(&pTexResource, (UINT)mWidth, (UINT)mHeight,
                                                                 DXGI_FORMAT_R8G8B8A8_UNORM, combinedImage);
        }
        
        if (result)
        {
            D3D12_RESOURCE_DESC             desc = pTexResource->GetDesc();
            D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
            SRVDesc.Format = desc.Format;
            SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            SRVDesc.Texture2D.MipLevels = desc.MipLevels;
            
            if (m_pResourceManager->AllocDescriptorTable(&srv, 1))
            {
                pD3DDevice->CreateShaderResourceView(pTexResource, &SRVDesc, srv.cpuHandle);

                pTexHandle = AllocTextureHandle();
                pTexHandle->pTexture = pTexResource;
                pTexHandle->IsFromFile = TRUE;
                pTexHandle->srv = srv;

                pTexHandle->pSearchHandle = m_pHashTable->Insert((void *)pTexHandle, metallicFilename, keySize);
                if (!pTexHandle->pSearchHandle)
                    __debugbreak();
            }
            else
            {
                pTexResource->Release();
                pTexResource = nullptr;
            }
        }

    lb_return:
        if (rImage)
        {
            delete[] rImage;
            rImage = nullptr;
        }
        if (mImage)
        {
            delete[] mImage;
            mImage = nullptr;
        }
        if (combinedImage)
        {
            delete[] combinedImage;
            combinedImage = nullptr;
        }
    }
    return pTexHandle;
}

void TextureManager::DeleteTexture(TEXTURE_HANDLE *pTexHandle)
{
    DeallocTextureHandle(pTexHandle);
}

TextureManager::~TextureManager()
{
    Cleanup();
}
