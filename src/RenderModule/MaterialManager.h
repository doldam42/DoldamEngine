#pragma once

#include "RendererTypedef.h"

class D3D12Renderer;

struct MATERIAL_HANDLE : IRenderMaterial
{
    static const UINT DESCRIPTOR_SIZE = 5;

    UINT  index;
    ULONG  refCount;
    void *pSysMemAddr = nullptr;
    void *pSearchHandle = nullptr;

    TEXTURE_HANDLE *pAlbedoTexHandle = nullptr;
    TEXTURE_HANDLE *pNormalTexHandle = nullptr;
    TEXTURE_HANDLE *pAOTexHandle = nullptr;
    TEXTURE_HANDLE *pEmissiveTexHandle = nullptr;
    TEXTURE_HANDLE *pMetallicRoughnessTexHandle = nullptr;

    inline void CopyDescriptors(ID3D12Device *pDevice, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, UINT descriptorSize) const
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE dest(cpuHandle);
        pDevice->CopyDescriptorsSimple(1, dest, pAlbedoTexHandle->srv.cpuHandle,
                                       D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        dest.Offset(descriptorSize);
        pDevice->CopyDescriptorsSimple(1, dest, pNormalTexHandle->srv.cpuHandle,
                                       D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        dest.Offset(descriptorSize);
        pDevice->CopyDescriptorsSimple(1, dest, pAOTexHandle->srv.cpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        dest.Offset(descriptorSize);
        pDevice->CopyDescriptorsSimple(1, dest, pMetallicRoughnessTexHandle->srv.cpuHandle,
                                       D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        dest.Offset(descriptorSize);
        pDevice->CopyDescriptorsSimple(1, dest, pEmissiveTexHandle->srv.cpuHandle,
                                       D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    // Inherited via IRenderMaterial
    BOOL UpdateTextureWithTexture(ITextureHandle *pTexture, TEXTURE_TYPE type) override;

    HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject) override;
    ULONG __stdcall AddRef(void) override;
    ULONG __stdcall Release(void) override;
};

class MaterialManager
{
    D3D12Renderer        *m_pRenderer = nullptr;
    D3D12ResourceManager *m_pResourceManager = nullptr;

    HashTable  *m_pHashTable = nullptr;
    MemoryPool *m_pMaterialCBPool = nullptr;
    MemoryPool *m_pMaterialHandlePool = nullptr;

    ID3D12Resource *m_pMatResource = nullptr;
    ID3D12Resource *m_pUploadBuffer = nullptr;
    UINT8          *m_pSystemMemAddr = nullptr;

    DESCRIPTOR_HANDLE m_srv = {};

    BOOL m_isUpdated = false;
    UINT m_sizePerMat = 0;
    UINT m_maxMatNum = 0;

  private:
    void InitMaterialTextures(MATERIAL_HANDLE *pOutMaterial, const Material *pInMaterial);
    void CleanupMaterial(MATERIAL_HANDLE *pMaterial);

    MATERIAL_HANDLE *AllocMaterialHandle(const Material *pMaterial);

    void Cleanup();

  public:
    bool Initialize(D3D12Renderer *pRenderer, UINT sizePerMat, UINT maxMatNum);

    MATERIAL_HANDLE *CreateMaterial(const void *pInMaterial, const wchar_t *pMaterialName);
    void             DeleteMaterial(MATERIAL_HANDLE *pMatHandle);

    BOOL UpdateMaterial(MATERIAL_HANDLE *pMatHandle, const Material *pInMaterial);
    BOOL UpdateMaterialTexture(MATERIAL_HANDLE *pMatHandle, TEXTURE_HANDLE* pTexHandle, TEXTURE_TYPE type);

    void Update(ID3D12GraphicsCommandList *pCommandList);

    D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const { return m_srv.cpuHandle; }

    MaterialManager() = default;
    ~MaterialManager();
};
