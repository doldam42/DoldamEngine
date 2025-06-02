#pragma once

class D3D12Renderer;
class PostProcessor
{
  private:
    D3D12Renderer *m_pRenderer = nullptr;
  public:
    BOOL Initialize(D3D12Renderer *pRenderer);

    void Draw(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList, D3D12_VIEWPORT *pViewport,
              D3D12_RECT *pScissorRect, D3D12_CPU_DESCRIPTOR_HANDLE src, D3D12_CPU_DESCRIPTOR_HANDLE renderTarget);

    PostProcessor() = default;
    ~PostProcessor();
};
