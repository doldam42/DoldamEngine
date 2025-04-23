#include "FrameBuffer.h"
#include "pch.h"

void FrameBuffer::Initialize(UINT itemSizePerByte, UINT maxItemCount)
{
    m_pBuffer = new BYTE[itemSizePerByte * maxItemCount];
    m_maxItemCount = maxItemCount;
    m_itemSize = itemSizePerByte;
}

void *FrameBuffer::Alloc(UINT numItem)
{
    if (m_allocedItemCount + numItem > m_maxItemCount)
    {
        DASSERT(false);
        return nullptr;
    }
    BYTE *pAlloced = m_pBuffer + (m_allocedItemCount * m_itemSize);
    m_allocedItemCount += numItem;
    return pAlloced;
}

void FrameBuffer::Reset() { m_allocedItemCount = 0; }

FrameBuffer::~FrameBuffer()
{
    if (m_pBuffer)
    {
        delete[] m_pBuffer;
        m_pBuffer = nullptr;
    }
}
