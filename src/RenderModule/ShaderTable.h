#pragma once

class ShaderTable
{
    ID3D12Resource *m_pShaderRecordsGPU = nullptr;
    uint8_t        *m_pShaderRecordsCPU = nullptr;

    UINT m_shaderRecordSize = 0;
    UINT m_maxRecordCount = 0;
    UINT m_allocatedRecordCount = 0;

    // Debug Support
    WCHAR m_name[MAX_NAME] = {};

    void Cleanup();

  public:
    ShaderTable() = default;
    void Initialize(ID3D12Device *pDevice, UINT numShaderRecords, UINT shaderRecordSize, const WCHAR *resourceName = nullptr);
    void InsertRecord(void *pShaderIdentifier, UINT shaderIdentifierSize, void *pLocalRootArguments,
                      UINT localRootArgumentsSize);
    void InsertRecord(void *pShaderIdentifier, UINT shaderIdentifierSize);

    UINT GetShaderRecordSize() const
    {
        return m_shaderRecordSize;
    }
    UINT GetAllocatedRecordCount() const
    {
        return m_allocatedRecordCount;
    }
    UINT GetSizeInBytes() const 
    {
        return m_allocatedRecordCount * m_shaderRecordSize;
    }
    ID3D12Resource *GetResource()
    {
        return m_pShaderRecordsGPU;
    }
    void Reset();

    ~ShaderTable();
};
