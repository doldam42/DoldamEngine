#pragma once

#include "RendererTypedef.h"

struct RENDER_MESH_OBJ_PARAM
{
    IRenderMaterial **ppMaterials;
    UINT              numMaterials;

    FILL_MODE fillMode;
    Matrix    worldTM;
};

struct RENDER_CHAR_OBJ_PARAM
{
    const Matrix     *pBones;
    IRenderMaterial **ppMaterials;
    UINT              numBones;
    UINT              numMaterials;

    FILL_MODE         fillMode;
    Matrix            worldTM;
};

struct RENDER_SPRITE_PARAM
{
    FILL_MODE fillMode;
    int       posX;
    int       posY;
    float     scaleX;
    float     scaleY;
    RECT      rect;
    BOOL      isUseRect;
    float     Z;

    ITextureHandle *pTexHandle;
};

struct RENDER_ITEM
{
    RENDER_ITEM_TYPE   type;
    IRenderableObject *pObjHandle;
    union {
        RENDER_CHAR_OBJ_PARAM charObjParam;
        RENDER_MESH_OBJ_PARAM meshObjParam;
        RENDER_SPRITE_PARAM   spriteParam;
    };
};

class CommandListPool;
class D3D12Renderer;
class RenderQueue
{
    D3D12Renderer *m_pRenderer = nullptr;

    char *m_pBuffer = nullptr;
    UINT  m_maxBufferSize = 0;
    UINT  m_allocatedSize = 0;
    UINT  m_readBufferPos = 0;
    UINT  m_itemCount = 0;

    const RENDER_ITEM *Dispatch();
    void               Cleanup();

  public:
    BOOL Initialize(D3D12Renderer *pRenderer, UINT maxItemNum);
    BOOL Add(const RENDER_ITEM *pItem);
    UINT Process(UINT threadIndex, CommandListPool *pCommandListPool, ID3D12CommandQueue *pCommandQueue,
                 DWORD processCountPerCommandList, D3D12_CPU_DESCRIPTOR_HANDLE rtv, D3D12_CPU_DESCRIPTOR_HANDLE dsv,
                 D3D12_GPU_DESCRIPTOR_HANDLE global, const D3D12_VIEWPORT *pViewport, const D3D12_RECT *pScissorRect,
                 UINT rtvCount, DRAW_PASS_TYPE passType);
    void Reset();
    void Revert();

    RenderQueue() = default;
    ~RenderQueue();
};
