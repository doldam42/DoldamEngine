#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <DirectXMath.h>

enum SPRITE_DESCRIPTOR_INDEX
{
    SPRITE_DESCRIPTOR_INDEX_CBV = 0,
    SPRITE_DESCRIPTOR_INDEX_TEX
};

class D3D12Renderer;
struct TEXTURE_HANDLE;
class SpriteObject : public IRenderSprite
{
  public:
    static const UINT DESCRIPTOR_COUNT_PER_DRAW = 2; // | Constant Buffer | TEX |

  private:
    static ULONG m_initRefCount;
    ULONG       m_refCount = 0;

    // vertex data
    static ID3D12Resource          *m_pVertexBuffer;
    static D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    // index data
    static ID3D12Resource *m_pIndexBuffer;
    static D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

    TEXTURE_HANDLE *m_pTexHandle = nullptr;
    D3D12Renderer  *m_pRenderer = nullptr;
    RECT            m_Rect = {};
    Vector2         m_scale = Vector2(1.0f);

    BOOL InitSharedResources();
    void CleanupSharedResources();

    BOOL InitMesh();

    void Cleanup();

  public:
    BOOL Initialize(D3D12Renderer *pRenderer);
    BOOL Initialize(D3D12Renderer *pRenderer, const WCHAR *texFileName, const RECT *pRect);
    void DrawWithTex(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList, const Vector2 *pPos, const Vector2 *pScale,
                     const RECT *pRect, float Z, TEXTURE_HANDLE *pTexHandle);
    void Draw(UINT threadIndex, ID3D12GraphicsCommandList *pCommandList, const Vector2 *pPos, const Vector2 *pScale,
              float Z);

    static const D3D12_VERTEX_BUFFER_VIEW &GetVertexBufferView()
    {
        return m_vertexBufferView;
    }
    static const D3D12_INDEX_BUFFER_VIEW &GetIndexBufferView()
    {
        return m_indexBufferView;
    }

    SpriteObject() = default;
    ~SpriteObject();

    // Inherited via IRenderSprite
    HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject) override;
    ULONG __stdcall AddRef(void) override;
    ULONG __stdcall Release(void) override;
};
