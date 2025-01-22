#pragma once

#include <DirectXMath.h>
#include <d3d12.h>
#include <d3dx12.h>

class D3D12Renderer;
struct TEXTURE_HANDLE;
class Terrain
{
  private:
    // vertex data
    ID3D12Resource          *m_pVertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    // index data
    ID3D12Resource         *m_pIndexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

    TEXTURE_HANDLE *m_pTexHandle = nullptr;
    D3D12Renderer  *m_pRenderer = nullptr;

    BOOL InitMesh();

    void Cleanup();

    public:
    BOOL Initialize(D3D12Renderer *pRenderer, const WCHAR* terrainFileName);
};
