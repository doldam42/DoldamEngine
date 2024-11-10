#pragma once

#include "RendererTypedef.h"

class D3D12Renderer;
class MaterialManager
{
    D3D12Renderer        *m_pRenderer = nullptr;
    D3D12ResourceManager *m_pResourceManager = nullptr;

    HashTable  *m_pHashTable = nullptr;
    MemoryPool *m_pMemoryPool = nullptr;

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
    UINT             DeallocMaterialHandle(MATERIAL_HANDLE *pMatHandle);

    void Cleanup();

  public:
    bool Initialize(D3D12Renderer *pRenderer, UINT sizePerMat, UINT maxMatNum);

    MATERIAL_HANDLE *CreateMaterial(const void *pInMaterial, const wchar_t *pMaterialName);
    void             DeleteMaterial(MATERIAL_HANDLE *pMatHandle);

    BOOL UpdateMaterial(MATERIAL_HANDLE *pMatHandle, const Material *pInMaterial);

    void Update(ID3D12GraphicsCommandList *pCommandList);

    D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const { return m_srv.cpuHandle; }

    MaterialManager() = default;
    ~MaterialManager();
};
