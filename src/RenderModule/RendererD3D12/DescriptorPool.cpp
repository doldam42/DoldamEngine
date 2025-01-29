#include "pch.h"

#include "DescriptorPool.h"

void DescriptorPool::Cleanup() {
  m_pDescriptorHeap->Release();
  m_pDescriptorHeap = nullptr;
}

BOOL DescriptorPool::Initialize(ID3D12Device5 *pD3DDevice,
                                UINT           maxDescriptorCount) {
  BOOL result = FALSE;
  m_pD3DDevice = pD3DDevice;

  m_maxDescriptorCount = maxDescriptorCount;
  m_commonDescriptorSize = m_pD3DDevice->GetDescriptorHandleIncrementSize(
      D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

  D3D12_DESCRIPTOR_HEAP_DESC frameHeapDesc = {};
  frameHeapDesc.NumDescriptors = m_maxDescriptorCount;
  frameHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  frameHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  if (FAILED(m_pD3DDevice->CreateDescriptorHeap(
          &frameHeapDesc, IID_PPV_ARGS(&m_pDescriptorHeap)))) {
    __debugbreak();
    goto lb_return;
  }

  m_pDescriptorHeap->SetName(L"FrameRate DescriptorHeap");

  result = TRUE;
lb_return:
  return result;
}

BOOL DescriptorPool::Alloc(D3D12_CPU_DESCRIPTOR_HANDLE *pOutCPUDescriptor,
                           D3D12_GPU_DESCRIPTOR_HANDLE *pOutGPUDescriptor,
                           UINT                         DescriptorCount) {
  if (m_AllocatedDescriptorCount + DescriptorCount > m_maxDescriptorCount)
    return FALSE;

  *pOutCPUDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(
      m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
      m_AllocatedDescriptorCount, m_commonDescriptorSize);
  *pOutGPUDescriptor = CD3DX12_GPU_DESCRIPTOR_HANDLE(
      m_pDescriptorHeap->GetGPUDescriptorHandleForHeapStart(),
      m_AllocatedDescriptorCount, m_commonDescriptorSize);
  m_AllocatedDescriptorCount += DescriptorCount;

  return TRUE;
}

void DescriptorPool::Reset() { m_AllocatedDescriptorCount = 0; }

ID3D12DescriptorHeap *DescriptorPool::GetDescriptorHeap() {
  return m_pDescriptorHeap;
}

DescriptorPool::~DescriptorPool() { Cleanup(); }
