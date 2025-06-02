#pragma once

#include "../Common/RendererInterface.h"
#include <d3d12.h>
#include <dwrite_3.h>

enum LIGHT_FLAGS : UINT
{
    LIGHT_OFF = 0x00,
    LIGHT_DIRECTIONAL = 0x01,
    LIGHT_POINT = 0x02,
    LIGHT_SPOT = 0x04,
    LIGHT_SHADOW = 0x10,
};

constexpr const UINT MAX_LIGHTS = 3;

constexpr const UINT MAX_RENDER_THREAD_COUNT = 8;
constexpr const UINT  SWAP_CHAIN_FRAME_COUNT = 3;
constexpr const UINT  MAX_PENDING_FRAME_COUNT = SWAP_CHAIN_FRAME_COUNT - 1;
constexpr const float STRENGTH_IBL = 0.2f;

enum SAMPLER_TYPE : UINT
{
    SAMPLER_TYPE_LINEAR_WRAP = 0,
    SAMPLER_TYPE_LINEAR_CLAMP,
    SAMPLER_TYPE_POINT_WRAP,
    SAMPLER_TYPE_POINT_CLAMP,
    SAMPLER_TYPE_SHADOW_POINT,
    SAMPLER_TYPE_ANISOTROPIC_WRAP,
    SAMPLER_TYPE_ANISOTROPIC_CLAMP,
    SAMPLER_TYPE_COUNT
};

enum RENDER_ITEM_TYPE : UINT
{
    RENDER_ITEM_TYPE_MESH_OBJ = 0,
    RENDER_ITEM_TYPE_CHAR_OBJ,
    RENDER_ITEM_TYPE_SPRITE,
    RENDER_ITEM_TYPE_TERRAIN,
    RENDER_ITEM_TYPE_COUNT
};

enum DRAW_PASS_TYPE : UINT
{
    DRAW_PASS_TYPE_DEFAULT = 0,
    DRAW_PASS_TYPE_DEFERRED,
    DRAW_PASS_TYPE_SHADOW,
    DRAW_PASS_TYPE_TRANSPARENCY,
    DRAW_PASS_TYPE_COUNT
};

enum FILL_MODE : UINT
{
    FILL_MODE_SOLID = 0,
    FILL_MODE_WIRED,
    FILL_MODE_COUNT
};

enum RENDER_TARGET_TYPE : UINT
{
    RENDER_TARGET_TYPE_BACK = 0,
    RENDER_TARGET_TYPE_INTERMEDIATE,
    RENDER_TARGET_TYPE_DEFERRED,
    RENDER_TARGET_TYPE_RAYTRACING,
    RENDER_TARGET_TYPE_TEXTURE,
    RENDER_TARGET_TYPE_COUNT,
};

struct DESCRIPTOR_HANDLE
{
    UINT                        descriptorCount;
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
};

struct FONT_HANDLE : public IFontHandle
{
    IDWriteTextFormat *pTextFormat;
    float              fontSize;
    WCHAR              fontFamilyName[512];
};

struct TEXTURE_HANDLE : public ITextureHandle
{
    ID3D12Resource   *pTexture;
    ID3D12Resource   *pUploadBuffer;
    DESCRIPTOR_HANDLE srv;
    BOOL              IsUpdated;
    BOOL              IsFromFile;
    UINT              refCount;
    void             *pSearchHandle;
};

// #DXR
struct AccelerationStructureBuffers
{
    ID3D12Resource *pScratch = nullptr;
    ID3D12Resource *pResult = nullptr;
    ID3D12Resource *pInstanceDesc = nullptr;
};

struct DECOMP_PROJ
{
    float rcp_m11;
    float rcp_m22;
    float m21;
    float m31;
    float m32;
    float m33;
    float m43;
    float Reserved0;
};

struct FragmentListNode
{
    UINT  next;
    float depth;
    UINT  color;
};

constexpr const wchar_t *DEFAULT_LOCALE_NAME = L"ko-kr";