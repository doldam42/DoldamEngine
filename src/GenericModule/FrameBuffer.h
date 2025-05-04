#pragma once
// Frame���� �ʱ�ȭ �Ǵ� ��ȸ�� �޸� ����
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
