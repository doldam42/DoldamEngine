#pragma once

#include "ConstantBuffers.h"

struct CB_CONTAINER {
  D3D12_CPU_DESCRIPTOR_HANDLE CBVHandle;
  D3D12_GPU_VIRTUAL_ADDRESS   pGPUMemAddr;
  UINT8                      *pSystemMemAddr;
};

class ConstantBufferPool {
  CB_CONTAINER *m_pCBContainerList = nullptr;

  ID3D12DescriptorHeap *m_pCBVHeap = nullptr;
  ID3D12Resource       *m_pResource = nullptr;
  UINT8                *m_pSystemMemAddr = nullptr;
  UINT                  m_sizePerCBV = 0;
  UINT                  m_maxCBVNum = 0;
  UINT                  m_AllocatedCBVNum = 0;
  CONSTANT_BUFFER_TYPE  m_type;

  void Cleanup();

public:
  BOOL Initialize(ID3D12Device *pD3DDevice, CONSTANT_BUFFER_TYPE type, UINT SizePerCBV, UINT MaxCBVNum,
                  D3D12_DESCRIPTOR_HEAP_FLAGS heapFlag);

  CB_CONTAINER *Alloc();
  CB_CONTAINER *Alloc(UINT numCB);
  void          Reset();

  ID3D12DescriptorHeap *INL_GetDescriptorHeap() const { return m_pCBVHeap; }
  ConstantBufferPool();
  ~ConstantBufferPool();
};
