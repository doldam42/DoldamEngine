#include "pch.h"

#include "D3D12Renderer.h"
#include "D3D12ResourceManager.h"
#include "MaterialManager.h"

void MaterialManager::InitMaterialTextures(MATERIAL_HANDLE *pOutMaterial, const Material *pInMaterial)
{
    wchar_t path[MAX_PATH];

    TEXTURE_HANDLE *pAlbedoTexHandle = nullptr;
    TEXTURE_HANDLE *pNormalTexHandle = nullptr;
    TEXTURE_HANDLE *pAOTexHandle = nullptr;
    TEXTURE_HANDLE *pMetallicRoughnessTexHandle = nullptr;
    TEXTURE_HANDLE *pEmissiveTexHandle = nullptr;

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

    if (!pAlbedoTexHandle)
    {
        pAlbedoTexHandle = m_pRenderer->GetDefaultTex();
    }
    if (!pNormalTexHandle)
    {
        pNormalTexHandle = m_pRenderer->GetDefaultTex();
    }
    if (!pAOTexHandle)
    {
        pAOTexHandle = m_pRenderer->GetDefaultTex();
    }
    if (!pEmissiveTexHandle)
    {
        pEmissiveTexHandle = m_pRenderer->GetDefaultTex();
    }
    if (!pMetallicRoughnessTexHandle)
    {
        pMetallicRoughnessTexHandle = m_pRenderer->GetDefaultTex();
    }

    pOutMaterial->pAlbedoTexHandle = pAlbedoTexHandle;
    pOutMaterial->pNormalTexHandle = pNormalTexHandle;
    pOutMaterial->pAOTexHandle = pAOTexHandle;
    pOutMaterial->pMetallicRoughnessTexHandle = pMetallicRoughnessTexHandle;
    pOutMaterial->pEmissiveTexHandle = pEmissiveTexHandle;
}

void MaterialManager::CleanupMaterial(MATERIAL_HANDLE *pMaterial)
{
    void *pAddr = m_pMemoryPool->GetAddressOf(pMaterial->index);
    if (pAddr)
    {
        m_pMemoryPool->Dealloc(pAddr);
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
}

MATERIAL_HANDLE *MaterialManager::AllocMaterialHandle(const Material *pMaterial)
{
    MATERIAL_HANDLE *pMatHandle = new MATERIAL_HANDLE;

    UINT8 *sysMemAddr = (UINT8 *)m_pMemoryPool->Alloc();
    if (!sysMemAddr)
    {
        __debugbreak();
        return nullptr;
    }

    pMatHandle->refCount = 1;
    pMatHandle->index = m_pMemoryPool->GetIndexOf(sysMemAddr);

    MaterialConstants materialCB;

    materialCB.albedo = pMaterial->albedo;
    materialCB.emissive = pMaterial->emissive;
    materialCB.metallicFactor = pMaterial->metallicFactor;
    materialCB.roughnessFactor = pMaterial->roughnessFactor;
    materialCB.opacityFactor = pMaterial->opacityFactor;
    materialCB.reflectionFactor = pMaterial->reflectionFactor;

    materialCB.useAlbedoMap = wcslen(pMaterial->albedoTextureName) == 0 ? FALSE : TRUE;
    materialCB.useAOMap = wcslen(pMaterial->aoTextureName) == 0 ? FALSE : TRUE;
    materialCB.useEmissiveMap = wcslen(pMaterial->emissiveTextureName) == 0 ? FALSE : TRUE;
    materialCB.useMetallicMap = wcslen(pMaterial->metallicTextureName) == 0 ? FALSE : TRUE;
    materialCB.useRoughnessMap = materialCB.useMetallicMap;
    materialCB.useNormalMap = wcslen(pMaterial->normalTextureName) == 0 ? FALSE : TRUE;

    memcpy(sysMemAddr, &materialCB, m_sizePerMat);

    pMatHandle->pSysMemAddr = sysMemAddr;

    InitMaterialTextures(pMatHandle, pMaterial);

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

    if (m_pMemoryPool)
    {
        delete m_pMemoryPool;
        m_pMemoryPool = nullptr;
    }
}

bool MaterialManager::Initialize(D3D12Renderer *pRenderer, UINT sizePerMat, UINT maxMatNum)
{
    bool            result = false;
    ID3D12Resource *pMatResource = nullptr;
    ID3D12Resource *pUploadBuffer = nullptr;

    ID3D12Device *pD3DDevice = pRenderer->INL_GetD3DDevice();

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

    pRenderer->INL_GetResourceManager()->AllocDescriptorTable(&m_srv, 1);
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

    m_pMemoryPool = new MemoryPool;
    m_pMemoryPool->Initialize(m_pSystemMemAddr, sizePerMat, maxMatNum);

    m_pRenderer = pRenderer;
    m_pResourceManager = pRenderer->INL_GetResourceManager();
    m_maxMatNum = maxMatNum;
    m_sizePerMat = sizePerMat;
    m_pMatResource = pMatResource;
    m_pUploadBuffer = pUploadBuffer;

    result = true;
lb_return:

    return result;
}

MATERIAL_HANDLE *MaterialManager::CreateMaterial(const void *pInMaterial, const wchar_t *pMaterialName)
{
    MATERIAL_HANDLE *pMatHandle = nullptr;
    UINT             keySize = wcslen(pMaterialName) * sizeof(wchar_t);

    if (m_pHashTable->Select((void **)&pMatHandle, 1, pMaterialName, keySize))
    {
        pMatHandle->refCount++;
    }
    else
    {
        pMatHandle = AllocMaterialHandle((Material *)pInMaterial);

        m_isUpdated = true;

        pMatHandle->pSearchHandle = m_pHashTable->Insert((void *)pMatHandle, pMaterialName, keySize);
    }

    return pMatHandle;
}

void MaterialManager::DeleteMaterial(MATERIAL_HANDLE *pMatHandle)
{
    if (pMatHandle)
    {
        CleanupMaterial(pMatHandle);
        delete pMatHandle;
    }
    m_isUpdated = true;
}

BOOL MaterialManager::UpdateMaterial(MATERIAL_HANDLE *pMatHandle, const Material *pInMaterial)
{
    memcpy(pMatHandle->pSysMemAddr, pInMaterial, m_sizePerMat);
    m_isUpdated = true;
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
