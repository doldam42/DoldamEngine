#pragma once

#include "RendererTypedef.h"

struct RENDER_MESH_OBJ_PARAM
{
    Matrix worldTM;

    IRenderMaterial *ppMaterials[12];
    Keyframe       **ppKeyframes;
    UINT numMaterials;
    UINT frameCount;
};

//
//struct RENDER_CHAR_OBJ_PARAM
//{
//    Matrix worldTM;
//
//    IRenderMaterial *ppMaterials[12];
//
//    UINT numMaterials;
//    
//};

struct RENDER_SPRITE_PARAM
{
    int   posX;
    int   posY;
    float scaleX;
    float scaleY;
    RECT  rect;
    BOOL  isUseRect;
    float Z;

    ITextureHandle *pTexHandle;
};

struct RENDER_TERRAIN_PARAM
{
    FILL_MODE fillMode;
    Vector3   scale;
};

struct RENDER_ITEM
{
    RENDER_ITEM_TYPE   type;
    FILL_MODE          fillMode;
    IRenderableObject *pObjHandle;
    union {
        //RENDER_CHAR_OBJ_PARAM charObjParam;
        RENDER_MESH_OBJ_PARAM meshObjParam;
        RENDER_SPRITE_PARAM   spriteParam;
        RENDER_TERRAIN_PARAM  terrainParam;
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
                 DWORD processCountPerCommandList, D3D12_CPU_DESCRIPTOR_HANDLE *rtvs, D3D12_CPU_DESCRIPTOR_HANDLE *dsv,
                 const D3D12_VIEWPORT *pViewport, const D3D12_RECT *pScissorRect, UINT rtvCount,
                 DRAW_PASS_TYPE passType);
    void Reset();
    void Revert();

    RenderQueue() = default;
    ~RenderQueue();
};
