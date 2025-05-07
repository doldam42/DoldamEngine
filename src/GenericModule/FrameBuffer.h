#pragma once
// Frame마다 초기화 되는 일회용 메모리 버퍼
class FrameBuffer
{
    BYTE *m_pBuffer = nullptr;

    UINT m_maxBufferSize = 0;
    UINT m_offset = 0;

  public:
    void  Initialize(UINT maxBufferSize);
    void *Alloc(UINT sizeInByte);
    void  Reset();

    FrameBuffer() = default;
    ~FrameBuffer();
};
