#include "pch.h"

#include "ConstantBufferPool.h"

void ConstantBufferPool::Cleanup() {
  if (m_pCBContainerList) {
    delete[] m_pCBContainerList;
    m_pCBContainerList = nullptr;
  }
  if (m_pResource) {
    m_pResource->Release();
    m_pResource = nullptr;
  }
  if (m_pCBVHeap) {
    m_pCBVHeap->Release();
    m_pCBVHeap = nullptr;
  }
}

BOOL ConstantBufferPool::Initialize(ID3D12Device *pD3DDevice, CONSTANT_BUFFER_TYPE type, UINT SizePerCBV,
                                    UINT MaxCBVNum, D3D12_DESCRIPTOR_HEAP_FLAGS heapFlag) {

  m_maxCBVNum = MaxCBVNum;
  m_sizePerCBV = SizePerCBV;
  m_type = type;

  UINT ByteWidth = SizePerCBV * m_maxCBVNum;

  // Create the Constant Buffer
  if (FAILED(pD3DDevice->CreateCommittedResource(
          &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
          D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(ByteWidth),
          D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
          IID_PPV_ARGS(&m_pResource)))) {
    __debugbreak();
  }

  // create descriptor heap
  D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
  heapDesc.NumDescriptors = m_maxCBVNum;
  heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  heapDesc.Flags = heapFlag;
  if (FAILED(pD3DDevice->CreateDescriptorHeap(&heapDesc,
                                              IID_PPV_ARGS(&m_pCBVHeap)))) {
    __debugbreak();
  }
  CD3DX12_RANGE writeRange(
      0, 0); // We do not intend to write from this resource on the CPU.
  m_pResource->Map(0, &writeRange,
                   reinterpret_cast<void **>(&m_pSystemMemAddr));

  m_pCBContainerList = new CB_CONTAINER[m_maxCBVNum];

  D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
  cbvDesc.BufferLocation = m_pResource->GetGPUVirtualAddress();
  cbvDesc.SizeInBytes = m_sizePerCBV;

  UINT8                        *pSystemMemPtr = m_pSystemMemAddr;
  CD3DX12_CPU_DESCRIPTOR_HANDLE heapHandle(
      m_pCBVHeap->GetCPUDescriptorHandleForHeapStart());

  UINT DescriptorSize = pD3DDevice->GetDescriptorHandleIncrementSize(
      D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  for (DWORD i = 0; i < m_maxCBVNum; i++) {
    pD3DDevice->CreateConstantBufferView(&cbvDesc, heapHandle);

    m_pCBContainerList[i].CBVHandle = heapHandle;
    m_pCBContainerList[i].pGPUMemAddr = cbvDesc.BufferLocation;
    m_pCBContainerList[i].pSystemMemAddr = pSystemMemPtr;

    heapHandle.Offset(1, DescriptorSize);
    cbvDesc.BufferLocation += m_sizePerCBV;
    pSystemMemPtr += m_sizePerCBV;
  }
  return TRUE;
}

CB_CONTAINER *ConstantBufferPool::Alloc() {
  CB_CONTAINER *pCB = nullptr;

  if (m_AllocatedCBVNum >= m_maxCBVNum)
    goto lb_return;

  pCB = m_pCBContainerList + m_AllocatedCBVNum;
  m_AllocatedCBVNum++;
lb_return:
  return pCB;
}

CB_CONTAINER *ConstantBufferPool::Alloc(UINT numCB)
{
    CB_CONTAINER *pCB = nullptr;

    if (m_AllocatedCBVNum >= m_maxCBVNum)
        goto lb_return;

    pCB = m_pCBContainerList + m_AllocatedCBVNum;
    m_AllocatedCBVNum += numCB;
lb_return:
    return pCB;
}

void ConstantBufferPool::Reset() { m_AllocatedCBVNum = 0; }

ConstantBufferPool::ConstantBufferPool() {}

ConstantBufferPool::~ConstantBufferPool() { Cleanup(); }
