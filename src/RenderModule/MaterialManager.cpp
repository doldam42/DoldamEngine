#include "pch.h"

#include "D3D12Renderer.h"
#include "D3D12ResourceManager.h"
#include "MaterialManager.h"

MATERIAL_HANDLE *MaterialManager::AllocMaterialHandle()
{
    UINT8 *sysMemAddr = (UINT8 *)m_pMemoryPool->Alloc();
    if (!sysMemAddr)
    {
        __debugbreak();
        return nullptr;
    }

    MATERIAL_HANDLE *pMatHandle = new MATERIAL_HANDLE;
    memset(pMatHandle, 0, sizeof(MATERIAL_HANDLE));
    pMatHandle->refCount = 1;
    pMatHandle->index = m_pMemoryPool->GetIndexOf(sysMemAddr);

    return pMatHandle;
}

UINT MaterialManager::DeallocMaterialHandle(MATERIAL_HANDLE *pMatHandle)
{
    if (!pMatHandle->refCount)
        __debugbreak();

    UINT refCount = --pMatHandle->refCount;
    if (!refCount)
    {
        void *pAddr = m_pMemoryPool->GetAddressOf(pMatHandle->index);
        if (pAddr)
        {
            m_pMemoryPool->Dealloc(pAddr);
            pMatHandle->index = UINT_MAX; // invalid index
        }

        if (pMatHandle->pSearchHandle)
        {
            m_pHashTable->Delete(pMatHandle->pSearchHandle);
            pMatHandle->pSearchHandle = nullptr;
        }

        delete pMatHandle;
    }
    return refCount;
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
        pMatHandle = AllocMaterialHandle();

        void *sysMemAddr = m_pMemoryPool->GetAddressOf(pMatHandle->index);
        memcpy(sysMemAddr, pInMaterial, m_sizePerMat);
        pMatHandle->pSysMemAddr = sysMemAddr;

        m_isUpdated = true;

        pMatHandle->pSearchHandle = m_pHashTable->Insert((void *)pMatHandle, pMaterialName, keySize);
    }

    return pMatHandle;
}

void MaterialManager::DeleteMaterial(MATERIAL_HANDLE *pMatHandle) { 
    DeallocMaterialHandle(pMatHandle); 
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
