#pragma once

#include "RenderThread.h"
class DescriptorAllocator {
  ID3D12Device5 *m_pD3DDevice = nullptr;
  UINT          *m_pIndexTable = nullptr;

  UINT m_blockSize = 0;
  UINT m_allocatedDescriptorCount = 0;
  UINT m_maxBlockCount = 0;
  UINT m_maxDescriptorCount = 0;
  UINT m_srvDescriptorSize = 0;

  ID3D12DescriptorHeap       *m_pDescritorHeap = nullptr;
  D3D12_CPU_DESCRIPTOR_HANDLE m_cpuDescriptorHandle = {};

  void Cleanup();

public:
  BOOL Initialize(ID3D12Device5 *pD3DDevice, UINT MaxDescriptorCount, UINT BlockSize,
                  D3D12_DESCRIPTOR_HEAP_TYPE heapType);

  BOOL Alloc(DESCRIPTOR_HANDLE *pOutDescriptor);
  void DeAlloc(DESCRIPTOR_HANDLE* pDescriptor);

  DescriptorAllocator() = default;
  ~DescriptorAllocator();
};
