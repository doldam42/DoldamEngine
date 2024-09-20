#pragma once

class HashTable;
class D3D12Renderer;
class D3D12ResourceManager;
class TextureManager
{
    D3D12Renderer        *m_pRenderer = nullptr;
    D3D12ResourceManager *m_pResourceManager = nullptr;
    HashTable            *m_pHashTable = nullptr;

    SORT_LINK *m_pTexLinkHead = nullptr;
    SORT_LINK *m_pTexLinkTail = nullptr;

    TEXTURE_HANDLE *AllocTextureHandle();
    UINT            DeallocTextureHandle(TEXTURE_HANDLE *pTexHandle);
    void            Cleanup();

  public:
    BOOL            Initialize(D3D12Renderer *pRenderer, UINT maxBucketNum, UINT maxFileNum);
    TEXTURE_HANDLE *CreateTextureFromFile(const WCHAR *filename, BOOL isCubemap);
    TEXTURE_HANDLE *CreateDynamicTexture(UINT texWidth, UINT texHeight);
    TEXTURE_HANDLE *CreateImmutableTexture(UINT texWidth, UINT texHeight, DXGI_FORMAT format, const BYTE *pInitImage);
    TEXTURE_HANDLE *CreateMetallicRoughnessTexture(const WCHAR *metallicFilename, const WCHAR *roughneessFilename);
    
    void            DeleteTexture(TEXTURE_HANDLE *pTexHandle);
    TextureManager() = default;
    ~TextureManager();
};
