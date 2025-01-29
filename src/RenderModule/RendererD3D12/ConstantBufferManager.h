#pragma once

#include "ConstantBuffers.h"

class ConstantBufferPool;
class ConstantBufferManager
{
    ConstantBufferPool *m_ppConstantBufferPool[CONSTANT_BUFFER_TYPE_COUNT];

  public:
    BOOL Initialize(ID3D12Device *pD3DDevice, UINT maxCBVNum);
    void Reset();

    ConstantBufferPool *GetConstantBufferPool(CONSTANT_BUFFER_TYPE type);

    ConstantBufferManager() = default;
    ~ConstantBufferManager();
};