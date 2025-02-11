#include "pch.h"

#include "SingleDescriptorAllocator.h"

void SingleDescriptorAllocator::Cleanup() 
{
    if (m_pDescriptorHeap)
    {
        m_pDescriptorHeap->Release();
        m_pDescriptorHeap = nullptr;
    }
}

BOOL SingleDescriptorAllocator::Initialize(ID3D12Device5 *pD3DDevice, UINT MaxDescriptorCount)
{ 
	BOOL result = FALSE;
    m_pD3DDevice = pD3DDevice;

	m_maxDescriptorCount = MaxDescriptorCount;
    m_srvDescriptorSize = m_pD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// create descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC commonHeapDesc = {};
    commonHeapDesc.NumDescriptors = m_maxDescriptorCount;
    commonHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    commonHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (FAILED(m_pD3DDevice->CreateDescriptorHeap(&commonHeapDesc, IID_PPV_ARGS(&m_pDescriptorHeap))))
    {
        __debugbreak();
        return FALSE;
    }
}

BOOL SingleDescriptorAllocator::Alloc(D3D12_CPU_DESCRIPTOR_HANDLE *pOutCPUHandle,
                                      D3D12_GPU_DESCRIPTOR_HANDLE *pOutGPUHandle)
{
    if (m_maxDescriptorCount < m_allocatedDescriptorCount)
    {
#ifdef _DEBUG
        __debugbreak();
#endif // _DEBUG
        return FALSE;
    }
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                                            m_allocatedDescriptorCount, m_srvDescriptorSize);
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(m_pDescriptorHeap->GetGPUDescriptorHandleForHeapStart(),
                                            m_allocatedDescriptorCount, m_srvDescriptorSize);

    *pOutCPUHandle = cpuHandle;
    *pOutGPUHandle = gpuHandle;

    m_allocatedDescriptorCount++;
}

SingleDescriptorAllocator::~SingleDescriptorAllocator() { Cleanup(); }
