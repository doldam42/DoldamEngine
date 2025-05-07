#pragma once
class D3D12Renderer;
class DebugLine
{
  private:
    ID3D12RootSignature *m_pRootSignature;
    ID3D12PipelineState *m_pPipelineState;

    ID3D12Device5 *m_pDevice = nullptr;
    D3D12Renderer *m_pRenderer = nullptr;

    ID3D12Resource          *m_pVertexBuffer = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};

    struct LineVertex
    {
        Vector3 position;
        RGBA    color;
    };
    LineVertex *m_pLineVertexList; // start, end
    UINT        m_drawLineCount = 0;
    UINT        m_maxLineCount = 0;

    void Cleanup();
    
    BOOL InitPiplineState();
    BOOL InitRootSignature();

  public:
    BOOL Initialize(D3D12Renderer *pRenderer, UINT maxLineCount);

    void DrawLine(const Vector3 &start, const Vector3 &end, const RGBA &color);

    void DrawLineAll(UINT threadIndex, ID3D12GraphicsCommandList4 *pCommandList);

    DebugLine() = default;
    ~DebugLine();
};
