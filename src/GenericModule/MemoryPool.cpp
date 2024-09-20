#include "pch.h"
#include "MemoryPool.h"
#include "pch.h"

void MemoryPool::Cleanup()
{
    if (m_pIndexTable)
    {
        delete[] m_pIndexTable;
        m_pIndexTable = nullptr;
    }
}

void MemoryPool::Initialize(void *pBaseAddr, UINT sizeInBytes, UINT maxItemNum)
{
    m_pBaseAddress = (uint8_t*)pBaseAddr;
    m_maxItemNum = maxItemNum;
    m_sizeInBytes = sizeInBytes;
    m_maxItemNum = maxItemNum;

    m_pIndexTable = new UINT[maxItemNum];
    for (UINT i = 0; i < maxItemNum; i++)
    {
        m_pIndexTable[i] = i;
    }
}

void *MemoryPool::Alloc()
{
    uint8_t *pOutAddr = m_pBaseAddress;
    if (m_allocatedItemCount + 1 > m_maxItemNum)
        return nullptr;

    uint8_t *pOutMemory = m_pBaseAddress;
    UINT i = m_pIndexTable[m_allocatedItemCount];

    pOutMemory += m_sizeInBytes * i;

    m_allocatedItemCount++;
    return pOutMemory;
}

void MemoryPool::Dealloc(void *pInAddr)
{
    UINT index = (UINT)(((uint8_t *)pInAddr - m_pBaseAddress) / m_sizeInBytes);
    m_pIndexTable[m_allocatedItemCount - 1] = index;
    m_allocatedItemCount--;
}

// return Address's Index In Memory Pool 
// If invalid Address return UINT_MAX
UINT MemoryPool::GetIndexOf(void *pInAddr)
{
    // return invalid index
    if (!pInAddr || (uint8_t *)pInAddr - m_pBaseAddress < 0)
        return UINT_MAX;

    return (UINT)(((uint8_t *)pInAddr - m_pBaseAddress) / m_sizeInBytes);
}

// return Index's Address In Memory Pool
// If invalid Index return nullptr
void *MemoryPool::GetAddressOf(UINT index)
{
    if (index > m_maxItemNum)
        return nullptr;
    return m_pBaseAddress + m_sizeInBytes * index;
}

MemoryPool::~MemoryPool()
{
    Cleanup();
}

