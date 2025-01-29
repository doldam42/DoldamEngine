#include "pch.h"

#include "ShaderTable.h"

// Helper to compute aligned buffer sizes
#ifndef ROUND_UP
#define ROUND_UP(v, powerOf2Alignment) (((v) + (powerOf2Alignment)-1) & ~((powerOf2Alignment)-1))
#endif

void ShaderTable::Cleanup()
{
    if (m_pShaderRecordsGPU)
    {
        if (m_pShaderRecordsCPU)
        {
            m_pShaderRecordsGPU->Unmap(0, nullptr);
            m_pShaderRecordsCPU = nullptr;
        }
        m_pShaderRecordsGPU->Release();
        m_pShaderRecordsGPU = nullptr;
    }
}

void ShaderTable::Initialize(ID3D12Device *pDevice, UINT numShaderRecords, UINT shaderRecordSize,
                             const WCHAR *resourceName)
{
    ID3D12Resource *pShaderTable = nullptr;
    wcscpy_s(m_name, resourceName);
    m_shaderRecordSize = ROUND_UP(shaderRecordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
    UINT bufferSize = numShaderRecords * m_shaderRecordSize;

    if (FAILED(pDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
                                                &CD3DX12_RESOURCE_DESC::Buffer(bufferSize), D3D12_RESOURCE_STATE_GENERIC_READ,
                                                nullptr, IID_PPV_ARGS(&pShaderTable))))
    {
        __debugbreak();
    }

    pShaderTable->Map(0, nullptr, reinterpret_cast<void **>(&m_pShaderRecordsCPU));
    m_maxRecordCount = numShaderRecords;
    m_pShaderRecordsGPU = pShaderTable;
}

void ShaderTable::InsertRecord(void *pShaderIdentifier, UINT shaderIdentifierSize, void *pLocalRootArguments,
                               UINT localRootArgumentsSize)
{
    if (m_allocatedRecordCount > m_maxRecordCount)
    {
        __debugbreak();
    }
    uint8_t *pDest = m_pShaderRecordsCPU + (m_shaderRecordSize * m_allocatedRecordCount);
    memcpy(pDest, pShaderIdentifier, shaderIdentifierSize);
    pDest += shaderIdentifierSize;
    memcpy(pDest, pLocalRootArguments, localRootArgumentsSize);
    m_allocatedRecordCount++;
}

void ShaderTable::InsertRecord(void *pShaderIdentifier, UINT shaderIdentifierSize)
{
    if (m_allocatedRecordCount > m_maxRecordCount)
    {
        __debugbreak();
    }
    uint8_t *pDest = m_pShaderRecordsCPU + (m_shaderRecordSize * m_allocatedRecordCount);
    memcpy(pDest, pShaderIdentifier, shaderIdentifierSize);
    m_allocatedRecordCount++;
}

void ShaderTable::Reset()
{
    m_allocatedRecordCount = 0;
}

ShaderTable::~ShaderTable()
{
    Cleanup();
}
