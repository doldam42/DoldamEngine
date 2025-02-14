#pragma once
class SingleDescriptorAllocator
{
    ID3D12Device5 *m_pD3DDevice = nullptr;
    ID3D12DescriptorHeap *m_pDescriptorHeap = nullptr;

    UINT m_srvDescriptorSize = 0;
    UINT m_allocatedDescriptorCount = 0;
    UINT m_maxDescriptorCount = 0;

    void Cleanup();

  public:
    BOOL Initialize(ID3D12Device5 *pD3DDevice, UINT MaxDescriptorCount);

    BOOL Alloc(D3D12_CPU_DESCRIPTOR_HANDLE *pOutCPUHandle, D3D12_GPU_DESCRIPTOR_HANDLE *pOutGPUHandle);

    ID3D12DescriptorHeap *GetDescriptorHeap() { return m_pDescriptorHeap; }

    SingleDescriptorAllocator() = default;
    ~SingleDescriptorAllocator();
};
