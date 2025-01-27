#pragma once

#include <DirectXMath.h>
#include <MathHeaders.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxcapi.h>

class D3D12Renderer;
struct TEXTURE_HANDLE;
struct MATERIAL_HANDLE;
class Terrain : public IRenderTerrain
{
  public:
    static const UINT DESCRIPTOR_COUNT_PER_DRAW = 8;

  private:
    D3D12Renderer           *m_pRenderer = nullptr;
    ID3D12Device            *m_pD3DDevice = nullptr;

    // vertex data
    ID3D12Resource          *m_pVertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    // index data
    ID3D12Resource         *m_pIndexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

    MATERIAL_HANDLE *m_pMaterialHandle = nullptr;

    UINT m_descriptorSize = 0;

    UINT m_vertexCount = 0;
    UINT m_indexCount = 0;

    UINT m_numSlices = 0;
    UINT m_numStacks = 0;
    Vector3 m_scale;

    ULONG m_refCount = 0;

    BOOL InitMesh(const WCHAR *heightFilename, const int numSlice, const int numStack);

    void Cleanup();

  public:
    BOOL Initialize(D3D12Renderer *pRenderer, const Vector3* pScale, const Material *pMaterial, const int numSlice = 1,
                    const int numStack = 1);
    void Draw(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList,
              D3D12_GPU_DESCRIPTOR_HANDLE globalCBV, DRAW_PASS_TYPE passType, const Vector3* pScale, FILL_MODE fillMode);
    Terrain() = default;
    ~Terrain();

    // Inherited via IRenderTerrain
    HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject) override;
    ULONG __stdcall AddRef(void) override;
    ULONG __stdcall Release(void) override;
};
