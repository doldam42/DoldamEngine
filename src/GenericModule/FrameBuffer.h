#pragma once
class FrameBuffer
{
    BYTE *m_pBuffer = nullptr;
    UINT  m_allocedItemCount = 0;
    UINT  m_maxItemCount = 0;
    UINT  m_itemSize = 0;

  public:
    void  Initialize(UINT itemSizePerByte, UINT maxItemCount);
    void *Alloc(UINT numItem);
    void  Reset();

    FrameBuffer() = default;
    ~FrameBuffer();
};
