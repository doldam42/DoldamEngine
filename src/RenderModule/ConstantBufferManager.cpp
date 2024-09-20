#include "pch.h"

#include "ConstantBufferPool.h"

#include "ConstantBufferManager.h"


CONSTANT_BUFFER_PROPERTY g_pConstantBufferPropList[] = {
    {CONSTANT_BUFFER_TYPE_GLOBAL, sizeof(GlobalConstants)},
    {CONSTANT_BUFFER_TYPE_MESH, sizeof(MeshConstants)},
    {CONSTANT_BUFFER_TYPE_SKINNED, sizeof(SkinnedConstants)},
    {CONSTANT_BUFFER_TYPE_SPRITE, sizeof(SpriteConstants)},
    {CONSTANT_BUFFER_TYPE_GEOMETRY, sizeof(GeometryConstants)},
};

BOOL ConstantBufferManager::Initialize(ID3D12Device *pD3DDevice, UINT maxCBVNum)
{
    for (UINT i = 0; i < CONSTANT_BUFFER_TYPE_COUNT; i++)
    {
        m_ppConstantBufferPool[i] = new ConstantBufferPool;
        m_ppConstantBufferPool[i]->Initialize(pD3DDevice, (CONSTANT_BUFFER_TYPE)i, g_pConstantBufferPropList[i].size,
                                              maxCBVNum, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    }
    return TRUE;
}

void ConstantBufferManager::Reset()
{
    for (UINT i = 0; i < CONSTANT_BUFFER_TYPE_COUNT; i++)
    {
        m_ppConstantBufferPool[i]->Reset();
    }
}

ConstantBufferPool *ConstantBufferManager::GetConstantBufferPool(CONSTANT_BUFFER_TYPE type)
{
    if (type >= CONSTANT_BUFFER_TYPE_COUNT)
        __debugbreak();
    return m_ppConstantBufferPool[type];
}

ConstantBufferManager::~ConstantBufferManager()
{
    for (UINT i = 0; i < CONSTANT_BUFFER_TYPE_COUNT; i++)
    {
        if (m_ppConstantBufferPool[i])
        {
            delete m_ppConstantBufferPool[i];
            m_ppConstantBufferPool[i] = nullptr;
        }
    }
}
