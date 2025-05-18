#pragma once

#include "RendererTypedef.h"

class D3D12Renderer;

enum DESCRIPTOR_INDEX_PER_MATERIAL
{
    DESCRIPTOR_INDEX_PER_MATERIAL_ALBEDO = 0,
    DESCRIPTOR_INDEX_PER_MATERIAL_NORMAL,
    DESCRIPTOR_INDEX_PER_MATERIAL_AO,
    DESCRIPTOR_INDEX_PER_MATERIAL_METALLIC_ROUGHNESS,
    DESCRIPTOR_INDEX_PER_MATERIAL_EMMISIVE,
    DESCRIPTOR_INDEX_PER_MATERIAL_HEIGHT,
    DESCRIPTOR_INDEX_PER_MATERIAL_COUNT
};

struct MATERIAL_HANDLE : IRenderMaterial
{
    static const UINT DESCRIPTOR_SIZE = DESCRIPTOR_INDEX_PER_MATERIAL_COUNT;

    UINT  index;
    ULONG  refCount;
    void *pSysMemAddr = nullptr;
    void *pSearchHandle = nullptr;

    MATERIAL_TYPE   type;
    TEXTURE_HANDLE *pAlbedoTexHandle = nullptr;
    TEXTURE_HANDLE *pNormalTexHandle = nullptr;
    TEXTURE_HANDLE *pAOTexHandle = nullptr;
    TEXTURE_HANDLE *pEmissiveTexHandle = nullptr;
    TEXTURE_HANDLE *pMetallicRoughnessTexHandle = nullptr;
    TEXTURE_HANDLE *pHeightTexHandle = nullptr;

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
        dest.Offset(descriptorSize);
        pDevice->CopyDescriptorsSimple(1, dest, pHeightTexHandle->srv.cpuHandle,
                                       D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    // Inherited via IRenderMaterial
    BOOL UpdateAlbedo(const Vector3 &albedo) override;
    BOOL UpdateMetallicRoughness(float metallic, float roughness) override;
    BOOL UpdateEmissive(const Vector3 &emmisive) override;

    BOOL UpdateTextureWithTexture(ITextureHandle *pTexture, TEXTURE_TYPE type) override;

    MATERIAL_TYPE GetType() override { return type; }

    ITextureHandle* GetTexture(TEXTURE_TYPE type) override
    {
        switch (type)
        {
        case TEXTURE_TYPE_ALBEDO:
            return pAlbedoTexHandle;
        case TEXTURE_TYPE_NORMAL:
            return pNormalTexHandle;
        case TEXTURE_TYPE_AO:
            return pAOTexHandle;
        case TEXTURE_TYPE_EMISSIVE:
            return pEmissiveTexHandle;
        case TEXTURE_TYPE_METALLIC_ROUGHNESS:
            return pMetallicRoughnessTexHandle;
        case TEXTURE_TYPE_HEIGHT:
            return pHeightTexHandle;
        default:
            return nullptr;
        }
    }

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
    void InitMaterialTextures(MATERIAL_HANDLE *pOutMaterial, MaterialConstants* pOutConsts, const Material *pInMaterial);
    void CleanupMaterial(MATERIAL_HANDLE *pMaterial);

    MATERIAL_HANDLE *AllocMaterialHandle(const Material *pMaterial);

    void Cleanup();

  public:
    bool Initialize(D3D12Renderer *pRenderer, UINT sizePerMat, UINT maxMatNum);

    MATERIAL_HANDLE *CreateMaterial(const Material *pInMaterial, MATERIAL_TYPE type);
    void             DeleteMaterial(MATERIAL_HANDLE *pMatHandle);

    BOOL UpdateMaterial(MATERIAL_HANDLE *pMatHandle, const Material *pInMaterial);
    BOOL UpdateMaterialAlbedo(MATERIAL_HANDLE *pMatHandle, const Vector3 &albedo);
    BOOL UpdateMaterialMetallicRoughness(MATERIAL_HANDLE *pMatHandle, float metallic, float roughness);
    BOOL UpdateMaterialEmmisive(MATERIAL_HANDLE *pMatHandle, const Vector3 &emisive);
    BOOL UpdateMaterialTexture(MATERIAL_HANDLE *pMatHandle, TEXTURE_HANDLE* pTexHandle, TEXTURE_TYPE type);

    void Update(ID3D12GraphicsCommandList *pCommandList);

    D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const { return m_srv.cpuHandle; }

    MaterialManager() = default;
    ~MaterialManager();
};
