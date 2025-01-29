#include "pch.h"

#include "DescriptorAllocator.h"

BOOL DescriptorAllocator::Initialize(ID3D12Device5 *pD3DDevice,
                                UINT MaxDescriptorCount, UINT BlockSize, D3D12_DESCRIPTOR_HEAP_TYPE heapType) {
  BOOL result = FALSE;
  m_pD3DDevice = pD3DDevice;

  m_blockSize = BlockSize;
  m_maxDescriptorCount = MaxDescriptorCount;
  m_maxBlockCount = MaxDescriptorCount / BlockSize;
  m_srvDescriptorSize = m_pD3DDevice->GetDescriptorHandleIncrementSize(
      D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

  // create descriptor heap
  D3D12_DESCRIPTOR_HEAP_DESC commonHeapDesc = {};
  commonHeapDesc.NumDescriptors = m_maxDescriptorCount;
  commonHeapDesc.Type = heapType;
  commonHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  if (FAILED(m_pD3DDevice->CreateDescriptorHeap(
          &commonHeapDesc, IID_PPV_ARGS(&m_pDescritorHeap)))) {
    __debugbreak();
    goto lb_return;
  }
  m_cpuDescriptorHandle =
      m_pDescritorHeap->GetCPUDescriptorHandleForHeapStart();

  m_pIndexTable = new UINT[m_maxBlockCount];
  for (UINT i = 0; i < m_maxBlockCount; i++)
    m_pIndexTable[i] = i;

  result = TRUE;
lb_return:
  return result;
}

BOOL DescriptorAllocator::Alloc(DESCRIPTOR_HANDLE *pOutDescriptor) {
  if (m_allocatedDescriptorCount + 1 > m_maxBlockCount)
    return FALSE;

  UINT i = m_pIndexTable[m_allocatedDescriptorCount];
  pOutDescriptor->cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
      m_cpuDescriptorHandle, m_srvDescriptorSize, i * m_blockSize);
  pOutDescriptor->descriptorCount = m_blockSize;

  m_allocatedDescriptorCount++;
  return TRUE;
}

void DescriptorAllocator::DeAlloc(DESCRIPTOR_HANDLE *pDescriptor) {
  SIZE_T offset = m_blockSize * m_srvDescriptorSize;
  UINT   index =
      (UINT)((pDescriptor->cpuHandle.ptr - m_cpuDescriptorHandle.ptr) / offset);
  m_pIndexTable[m_allocatedDescriptorCount - 1] = index;
  m_allocatedDescriptorCount--;
}

void DescriptorAllocator::Cleanup() {
  if (m_pDescritorHeap) {
    m_pDescritorHeap->Release();
    m_pDescritorHeap = nullptr;
  }
  delete[] m_pIndexTable;
}

DescriptorAllocator::~DescriptorAllocator() { Cleanup(); }
