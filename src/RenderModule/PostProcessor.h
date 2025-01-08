#pragma once

class D3D12Renderer;
class PostProcessor
{
  private:
    D3D12Renderer *m_pRenderer = nullptr;

    D3D12_VIEWPORT m_Viewport = {};
    D3D12_RECT     m_ScissorRect = {};

    // vertex data
    ID3D12Resource          *m_pVertexBuffer = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};

    // index data
    ID3D12Resource         *m_pIndexBuffer = nullptr;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};

    BOOL InitMesh();

    void Cleanup();

    public:
    BOOL Initialize(D3D12Renderer *pRenderer);

    void Draw(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList, D3D12_CPU_DESCRIPTOR_HANDLE src,
              D3D12_CPU_DESCRIPTOR_HANDLE renderTarget);

    PostProcessor() = default;
    ~PostProcessor();
};
