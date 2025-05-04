#include "pch.h"

#include "FrameBuffer.h"

void FrameBuffer::Initialize(UINT maxBufferSize) 
{
    m_pBuffer = new BYTE[maxBufferSize];
    m_maxBufferSize = maxBufferSize;
}

void *FrameBuffer::Alloc(UINT sizeInByte)
{
    if (m_offset + sizeInByte > m_maxBufferSize)
    {
        DASSERT(false);
        return nullptr;
    }

    BYTE *pAlloced = m_pBuffer + m_offset;
    m_offset += sizeInByte;
    return pAlloced;
}

void FrameBuffer::Reset() { m_offset = 0; }

FrameBuffer::~FrameBuffer()
{
    if (m_pBuffer)
    {
        delete[] m_pBuffer;
        m_pBuffer = nullptr;
    }
}
