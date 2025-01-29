#pragma once

enum DESCRIPOTR_INDEX_PER_CUBEMAP
{
    DESCRIPOTR_INDEX_PER_CUBEMAP_ENV = 0,
    DESCRIPOTR_INDEX_PER_CUBEMAP_SPECULAR,
    DESCRIPOTR_INDEX_PER_CUBEMAP_IRRADIANCE,
    DESCRIPOTR_INDEX_PER_CUBEMAP_BRDF,
    DESCRIPOTR_INDEX_PER_CUBEMAP_COUNT
};

class D3D12Renderer;
class Cubemap
{
    D3D12Renderer *m_pRenderer = nullptr;

    ID3D12Resource *m_pVertexBuffer = nullptr;
    ID3D12Resource *m_pIndexBuffer = nullptr;

    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};
    D3D12_INDEX_BUFFER_VIEW  m_indexBufferView = {};

    TEXTURE_HANDLE *m_pEnv = nullptr;
    TEXTURE_HANDLE *m_pSpecular = nullptr;
    TEXTURE_HANDLE *m_pIrradiance = nullptr;
    TEXTURE_HANDLE *m_pBrdf = nullptr;

    BOOL InitMesh();
    void Cleanup();

  public:
    void Initialize(D3D12Renderer *pRnd, const WCHAR *envFilename, const WCHAR *specularFilename,
                    const WCHAR *irradianceFilename, const WCHAR *brdfFilename);

    D3D12_CPU_DESCRIPTOR_HANDLE GetEnvSRV();
    D3D12_CPU_DESCRIPTOR_HANDLE GetSpecularSRV();
    D3D12_CPU_DESCRIPTOR_HANDLE GetIrradianceSRV();
    D3D12_CPU_DESCRIPTOR_HANDLE GetBrdfSRV();

    inline void CopyDescriptors(ID3D12Device *pDevice, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
                                UINT descriptorSize) const;

    void Draw(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList);

    Cubemap() = default;
    ~Cubemap();
};

void Cubemap::CopyDescriptors(ID3D12Device *pDevice, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, UINT descriptorSize) const
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE dest(cpuHandle);
    pDevice->CopyDescriptorsSimple(1, dest, m_pEnv->srv.cpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    dest.Offset(descriptorSize);
    pDevice->CopyDescriptorsSimple(1, dest, m_pSpecular->srv.cpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    dest.Offset(descriptorSize);
    pDevice->CopyDescriptorsSimple(1, dest, m_pIrradiance->srv.cpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    dest.Offset(descriptorSize);
    pDevice->CopyDescriptorsSimple(1, dest, m_pBrdf->srv.cpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}