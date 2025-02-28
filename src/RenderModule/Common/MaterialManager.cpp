#include "pch.h"

#ifdef RENDERER_RAYTRACING
#include "../RendererRaytracing/D3D12Renderer.h"
#elif defined(RENDERER_D3D12)
#include "../RendererD3D12/D3D12Renderer.h"
#endif

#include "D3D12ResourceManager.h"
#include "MaterialManager.h"

void MaterialManager::InitMaterialTextures(MATERIAL_HANDLE *pOutMaterial, MaterialConstants *pOutConsts,
                                           const Material *pInMaterial)
{
    wchar_t path[MAX_PATH];

    TEXTURE_HANDLE *pAlbedoTexHandle = nullptr;
    TEXTURE_HANDLE *pNormalTexHandle = nullptr;
    TEXTURE_HANDLE *pAOTexHandle = nullptr;
    TEXTURE_HANDLE *pMetallicRoughnessTexHandle = nullptr;
    TEXTURE_HANDLE *pEmissiveTexHandle = nullptr;
    TEXTURE_HANDLE *pHeightTexHandle = nullptr;

    // Albedo
    memset(path, 0, sizeof(path));
    wcscpy_s(path, pInMaterial->basePath);
    wcscat_s(path, pInMaterial->albedoTextureName);
    pAlbedoTexHandle = (TEXTURE_HANDLE *)m_pRenderer->CreateTextureFromFile(path);

    // Normal
    memset(path, 0, sizeof(path));
    wcscpy_s(path, pInMaterial->basePath);
    wcscat_s(path, pInMaterial->normalTextureName);
    pNormalTexHandle = (TEXTURE_HANDLE *)m_pRenderer->CreateTextureFromFile(path);

    // AO
    memset(path, 0, sizeof(path));
    wcscpy_s(path, pInMaterial->basePath);
    wcscat_s(path, pInMaterial->aoTextureName);
    pAOTexHandle = (TEXTURE_HANDLE *)m_pRenderer->CreateTextureFromFile(path);

    // Emissive
    memset(path, 0, sizeof(path));
    wcscpy_s(path, pInMaterial->basePath);
    wcscat_s(path, pInMaterial->emissiveTextureName);
    pEmissiveTexHandle = (TEXTURE_HANDLE *)m_pRenderer->CreateTextureFromFile(path);

    // Metallic-Roughness
    WCHAR metallicPath[MAX_PATH];
    WCHAR roughnessPath[MAX_PATH];
    memset(metallicPath, 0, sizeof(metallicPath));
    memset(roughnessPath, 0, sizeof(roughnessPath));

    wcscpy_s(metallicPath, pInMaterial->basePath);
    wcscpy_s(roughnessPath, pInMaterial->basePath);

    wcscat_s(metallicPath, pInMaterial->metallicTextureName);
    wcscat_s(roughnessPath, pInMaterial->roughnessTextureName);

    pMetallicRoughnessTexHandle =
        static_cast<TEXTURE_HANDLE *>(m_pRenderer->CreateMetallicRoughnessTexture(metallicPath, roughnessPath));

    // Height
    memset(path, 0, sizeof(path));
    wcscpy_s(path, pInMaterial->basePath);
    wcscat_s(path, pInMaterial->heightTextureName);
    pHeightTexHandle = (TEXTURE_HANDLE *)m_pRenderer->CreateTextureFromFile(path);

    pOutConsts->flags = ~0;
    if (!pAlbedoTexHandle)
    {
        pAlbedoTexHandle = m_pRenderer->GetDefaultTex();
    }
    if (!pNormalTexHandle)
    {
        pNormalTexHandle = m_pRenderer->GetDefaultTex();
        pOutConsts->flags &= ~MATERIAL_USE_NORMAL_MAP;
    }
    if (!pAOTexHandle)
    {
        pAOTexHandle = m_pRenderer->GetDefaultTex();
        pOutConsts->flags &= ~MATERIAL_USE_AO_MAP;
    }
    if (!pEmissiveTexHandle)
    {
        pEmissiveTexHandle = m_pRenderer->GetDefaultTex();
        pOutConsts->flags &= ~MATERIAL_USE_EMISSIVE_MAP;
    }
    if (!pMetallicRoughnessTexHandle)
    {
        pMetallicRoughnessTexHandle = m_pRenderer->GetDefaultTex();
        pOutConsts->flags &= ~(MATERIAL_USE_METALLIC_MAP | MATERIAL_USE_ROUGHNESS_MAP);
    }
    if (!pHeightTexHandle)
    {
        pHeightTexHandle = m_pRenderer->GetDefaultTex();
        pOutConsts->flags &= ~MATERIAL_USE_HEIGHT_MAP;
    }

    pOutMaterial->pAlbedoTexHandle = pAlbedoTexHandle;
    pOutMaterial->pNormalTexHandle = pNormalTexHandle;
    pOutMaterial->pAOTexHandle = pAOTexHandle;
    pOutMaterial->pMetallicRoughnessTexHandle = pMetallicRoughnessTexHandle;
    pOutMaterial->pEmissiveTexHandle = pEmissiveTexHandle;
    pOutMaterial->pHeightTexHandle = pHeightTexHandle;
}

void MaterialManager::CleanupMaterial(MATERIAL_HANDLE *pMaterial)
{
    void *pAddr = m_pMaterialCBPool->GetAddressOf(pMaterial->index);
    if (pAddr)
    {
        m_pMaterialCBPool->Dealloc(pAddr);
        pMaterial->index = UINT_MAX; // invalid index
    }

    if (pMaterial->pSearchHandle)
    {
        m_pHashTable->Delete(pMaterial->pSearchHandle);
        pMaterial->pSearchHandle = nullptr;
    }
    if (pMaterial->pAlbedoTexHandle)
    {
        m_pRenderer->DeleteTexture(pMaterial->pAlbedoTexHandle);
        pMaterial->pAlbedoTexHandle = nullptr;
    }
    if (pMaterial->pNormalTexHandle)
    {
        m_pRenderer->DeleteTexture(pMaterial->pNormalTexHandle);
        pMaterial->pNormalTexHandle = nullptr;
    }
    if (pMaterial->pAOTexHandle)
    {
        m_pRenderer->DeleteTexture(pMaterial->pAOTexHandle);
        pMaterial->pAOTexHandle = nullptr;
    }
    if (pMaterial->pMetallicRoughnessTexHandle)
    {
        m_pRenderer->DeleteTexture(pMaterial->pMetallicRoughnessTexHandle);
        pMaterial->pMetallicRoughnessTexHandle = nullptr;
    }
    if (pMaterial->pEmissiveTexHandle)
    {
        m_pRenderer->DeleteTexture(pMaterial->pEmissiveTexHandle);
        pMaterial->pEmissiveTexHandle = nullptr;
    }
    if (pMaterial->pHeightTexHandle)
    {
        m_pRenderer->DeleteTexture(pMaterial->pHeightTexHandle);
        pMaterial->pHeightTexHandle = nullptr;
    }
}

MATERIAL_HANDLE *MaterialManager::AllocMaterialHandle(const Material *pMaterial)
{
    MATERIAL_HANDLE  tmp;
    void            *pMemory = m_pMaterialHandlePool->Alloc();
    MATERIAL_HANDLE *pMatHandle = reinterpret_cast<MATERIAL_HANDLE *>(pMemory);
    memcpy(pMatHandle, &tmp, sizeof(MATERIAL_HANDLE)); // 함수 테이블 카피용

    UINT8 *sysMemAddr = (UINT8 *)m_pMaterialCBPool->Alloc();
    if (!sysMemAddr)
    {
        __debugbreak();
        return nullptr;
    }

    pMatHandle->refCount = 1;
    pMatHandle->index = m_pMaterialCBPool->GetIndexOf(sysMemAddr);

    MaterialConstants materialCB;

    materialCB.albedo = pMaterial->albedo;
    materialCB.emissive = pMaterial->emissive;
    materialCB.metallicFactor = pMaterial->metallicFactor;
    materialCB.roughnessFactor = pMaterial->roughnessFactor;
    // materialCB.opacityFactor = pMaterial->opacityFactor;
    materialCB.opacityFactor = 1.0f;
    materialCB.reflectionFactor = pMaterial->reflectionFactor;

    InitMaterialTextures(pMatHandle, &materialCB, pMaterial);

    memcpy(sysMemAddr, &materialCB, m_sizePerMat);

    pMatHandle->pSysMemAddr = sysMemAddr;

    return pMatHandle;
}

void MaterialManager::Cleanup()
{
    if (m_srv.descriptorCount > 0)
    {
        m_pResourceManager->DeallocDescriptorTable(&m_srv);
        m_srv = {};
    }

    if (m_pMatResource)
    {
        m_pMatResource->Release();
        m_pMatResource = nullptr;
    }

    if (m_pUploadBuffer)
    {
        if (m_pSystemMemAddr)
        {
            m_pUploadBuffer->Unmap(0, nullptr);
            m_pSystemMemAddr = nullptr;
        }
        m_pUploadBuffer->Release();
        m_pUploadBuffer = nullptr;
    }

    if (m_pHashTable)
    {
        delete m_pHashTable;
        m_pHashTable = nullptr;
    }

    if (m_pMaterialCBPool)
    {
        delete m_pMaterialCBPool;
        m_pMaterialCBPool = nullptr;
    }

    if (m_pMaterialHandlePool)
    {
        delete m_pMaterialHandlePool;
        m_pMaterialHandlePool = nullptr;
    }
}

bool MaterialManager::Initialize(D3D12Renderer *pRenderer, UINT sizePerMat, UINT maxMatNum)
{
    bool            result = false;
    ID3D12Resource *pMatResource = nullptr;
    ID3D12Resource *pUploadBuffer = nullptr;

    ID3D12Device *pD3DDevice = pRenderer->GetD3DDevice();

    // Create Material Resource
    if (FAILED(pD3DDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(sizePerMat * maxMatNum), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, nullptr,
            IID_PPV_ARGS(&pMatResource))))
    {
        __debugbreak();
        goto lb_return;
    }
    // Create Upload Buffer
    if (FAILED(pD3DDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(sizePerMat * maxMatNum), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&pUploadBuffer))))
    {
        __debugbreak();
        goto lb_return;
    }

    pUploadBuffer->Map(0, nullptr, reinterpret_cast<void **>(&m_pSystemMemAddr));

    pRenderer->GetResourceManager()->AllocDescriptorTable(&m_srv, 1);
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    srvDesc.Buffer.NumElements = maxMatNum;
    srvDesc.Buffer.StructureByteStride = sizePerMat;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    pD3DDevice->CreateShaderResourceView(pMatResource, &srvDesc, m_srv.cpuHandle);

    m_pHashTable = new HashTable();
    m_pHashTable->Initialize(13, sizePerMat, maxMatNum); // TODO: 최적의 버킷 개수 정하기

    m_pMaterialCBPool = new MemoryPool;
    m_pMaterialCBPool->Initialize(m_pSystemMemAddr, sizePerMat, maxMatNum);

    m_pMaterialHandlePool = new MemoryPool;
    m_pMaterialHandlePool->Initialize(sizeof(MATERIAL_HANDLE), maxMatNum);

    m_pRenderer = pRenderer;
    m_pResourceManager = pRenderer->GetResourceManager();
    m_maxMatNum = maxMatNum;
    m_sizePerMat = sizePerMat;
    m_pMatResource = pMatResource;
    m_pUploadBuffer = pUploadBuffer;

    result = true;
lb_return:

    return result;
}

MATERIAL_HANDLE *MaterialManager::CreateMaterial(const Material *pInMaterial)
{
    MATERIAL_HANDLE *pMatHandle = nullptr;

    Material         m;
    const WCHAR     *materialName;
    if (!pInMaterial)
    {
        materialName = L"default";
    }
    else
    {
        materialName = pInMaterial->name;
        m = *pInMaterial;
    }

    UINT keySize = wcslen(materialName) * sizeof(wchar_t);
    if (m_pHashTable->Select((void **)&pMatHandle, 1, materialName, keySize))
    {
        pMatHandle->refCount++;
    }
    else
    {
        pMatHandle = AllocMaterialHandle(&m);

        m_isUpdated = true;

        pMatHandle->pSearchHandle = m_pHashTable->Insert((void *)pMatHandle, materialName, keySize);
    }

    return pMatHandle;
}

void MaterialManager::DeleteMaterial(MATERIAL_HANDLE *pMatHandle)
{
    if (pMatHandle)
    {
        CleanupMaterial(pMatHandle);
        m_pMaterialHandlePool->Dealloc(pMatHandle);
    }
    m_isUpdated = true;
}

BOOL MaterialManager::UpdateMaterial(MATERIAL_HANDLE *pMatHandle, const Material *pInMaterial)
{
    if (!pMatHandle)
        return FALSE;

    CleanupMaterial(pMatHandle);
    MATERIAL_HANDLE *pNew = CreateMaterial(pInMaterial);
    *pMatHandle = *pNew;
    m_pMaterialHandlePool->Dealloc(pNew);

    m_isUpdated = true;
    return TRUE;
}

BOOL MaterialManager::UpdateMaterialAlbedo(MATERIAL_HANDLE *pMatHandle, const Vector3 &albedo)
{
    MaterialConstants *pMatConst = (MaterialConstants *)pMatHandle->pSysMemAddr;

    pMatConst->albedo = albedo;

    m_isUpdated = TRUE;
    return TRUE;
}

BOOL MaterialManager::UpdateMaterialMetallicRoughness(MATERIAL_HANDLE *pMatHandle, float metallic, float roughness)
{
    MaterialConstants *pMatConst = (MaterialConstants *)pMatHandle->pSysMemAddr;

    pMatConst->metallicFactor = metallic;
    pMatConst->roughnessFactor = roughness;

    m_isUpdated = TRUE;
    return TRUE;
}

BOOL MaterialManager::UpdateMaterialEmmisive(MATERIAL_HANDLE *pMatHandle, const Vector3 &emisive)
{
    MaterialConstants *pMatConst = (MaterialConstants *)pMatHandle->pSysMemAddr;

    pMatConst->emissive = emisive;

    m_isUpdated = TRUE;
    return TRUE;
}

BOOL MaterialManager::UpdateMaterialTexture(MATERIAL_HANDLE *pMatHandle, TEXTURE_HANDLE *pTexHandle, TEXTURE_TYPE type)
{
    MaterialConstants *pMatConst = (MaterialConstants *)pMatHandle->pSysMemAddr;
    switch (type)
    {
    case TEXTURE_TYPE_ALBEDO:
        if (pMatHandle->pAlbedoTexHandle)
        {
            m_pRenderer->DeleteTexture(pMatHandle->pAlbedoTexHandle);
            pMatHandle->pAlbedoTexHandle = nullptr;
        }
        pMatHandle->pAlbedoTexHandle = pTexHandle;
        pMatConst->flags |= MATERIAL_USE_ALBEDO_MAP;
        break;
    case TEXTURE_TYPE_NORMAL:
        if (pMatHandle->pNormalTexHandle)
        {
            m_pRenderer->DeleteTexture(pMatHandle->pNormalTexHandle);
            pMatHandle->pNormalTexHandle = nullptr;
        }
        pMatHandle->pNormalTexHandle = pTexHandle;
        pMatConst->flags |= MATERIAL_USE_NORMAL_MAP;
        break;
    case TEXTURE_TYPE_AO:
        if (pMatHandle->pAOTexHandle)
        {
            m_pRenderer->DeleteTexture(pMatHandle->pAOTexHandle);
            pMatHandle->pAOTexHandle = nullptr;
        }
        pMatHandle->pAOTexHandle = pTexHandle;
        pMatConst->flags |= MATERIAL_USE_AO_MAP;
        break;
    case TEXTURE_TYPE_EMISSIVE:
        if (pMatHandle->pEmissiveTexHandle)
        {
            m_pRenderer->DeleteTexture(pMatHandle->pEmissiveTexHandle);
            pMatHandle->pEmissiveTexHandle = nullptr;
        }
        pMatHandle->pEmissiveTexHandle = pTexHandle;
        pMatConst->flags |= MATERIAL_USE_EMISSIVE_MAP;
        break;
    case TEXTURE_TYPE_METALLIC_ROUGHNESS:
        if (pMatHandle->pMetallicRoughnessTexHandle)
        {
            m_pRenderer->DeleteTexture(pMatHandle->pMetallicRoughnessTexHandle);
            pMatHandle->pMetallicRoughnessTexHandle = nullptr;
        }
        pMatHandle->pMetallicRoughnessTexHandle = pTexHandle;
        pMatConst->flags |= (MATERIAL_USE_METALLIC_MAP | MATERIAL_USE_ROUGHNESS_MAP);
        break;
    default:
#ifdef _DEBUG
        __debugbreak();
#endif // _DEBUG
        return FALSE;
    }

    m_isUpdated = TRUE;
    return TRUE;
}

void MaterialManager::Update(ID3D12GraphicsCommandList *pCommandList)
{
    if (!m_isUpdated)
        return;

    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pMatResource,
                                                                           D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
                                                                           D3D12_RESOURCE_STATE_COPY_DEST));
    pCommandList->CopyResource(m_pMatResource, m_pUploadBuffer);
    pCommandList->ResourceBarrier(1,
                                  &CD3DX12_RESOURCE_BARRIER::Transition(m_pMatResource, D3D12_RESOURCE_STATE_COPY_DEST,
                                                                        D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE));
}

MaterialManager::~MaterialManager() { Cleanup(); }

BOOL MATERIAL_HANDLE::UpdateAlbedo(const Vector3 &albedo)
{
    MaterialManager *pMaterialManager = g_pRenderer->GetMaterialManager();
    BOOL             result = pMaterialManager->UpdateMaterialAlbedo(this, albedo);
    return result;
}

BOOL MATERIAL_HANDLE::UpdateMetallicRoughness(float metallic, float roughness)
{
    MaterialManager *pMaterialManager = g_pRenderer->GetMaterialManager();
    BOOL             result = pMaterialManager->UpdateMaterialMetallicRoughness(this, metallic, roughness);
    return result;
}

BOOL MATERIAL_HANDLE::UpdateEmissive(const Vector3 &emmisive)
{
    MaterialManager *pMaterialManager = g_pRenderer->GetMaterialManager();
    BOOL             result = pMaterialManager->UpdateMaterialEmmisive(this, emmisive);
    return result;
}

BOOL MATERIAL_HANDLE::UpdateTextureWithTexture(ITextureHandle *pTexture, TEXTURE_TYPE type)
{
    MaterialManager *pMaterialManager = g_pRenderer->GetMaterialManager();
    BOOL             result = pMaterialManager->UpdateMaterialTexture(this, (TEXTURE_HANDLE *)pTexture, type);
    return result;
}

HRESULT __stdcall MATERIAL_HANDLE::QueryInterface(REFIID riid, void **ppvObject) { return E_NOTIMPL; }

ULONG __stdcall MATERIAL_HANDLE::AddRef(void) { return ++refCount; }

ULONG __stdcall MATERIAL_HANDLE::Release(void)
{
    ULONG newRefCount = --refCount;
    if (newRefCount == 0)
    {
        g_pRenderer->WaitForGPU();
        g_pRenderer->DeleteMaterialHandle(this);
        return 0;
    }
    return newRefCount;
}
