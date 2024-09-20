#pragma once
class DescriptorPool {
  ID3D12Device5 *m_pD3DDevice = nullptr;
  ID3D12DescriptorHeap *m_pDescriptorHeap = nullptr;

  UINT m_commonDescriptorSize = 0;
  UINT m_AllocatedDescriptorCount = 0;
  UINT m_maxDescriptorCount = 0;

  void Cleanup();

public:
  BOOL Initialize(ID3D12Device5 *pD3DDevice, UINT maxDescriptorCount);
  BOOL Alloc(D3D12_CPU_DESCRIPTOR_HANDLE *pOutCPUDescriptor,
             D3D12_GPU_DESCRIPTOR_HANDLE *pOutGPUDescriptor,
             UINT                         DescriptorCount);
  void Reset();

  ID3D12DescriptorHeap *GetDescriptorHeap();

  DescriptorPool() = default;
  ~DescriptorPool();
};
