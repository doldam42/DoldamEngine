#pragma once

#include "RendererTypedef.h"

// Helper to compute aligned buffer sizes
#ifndef ROUND_UP
#define ROUND_UP(v, powerOf2Alignment) (((v) + (powerOf2Alignment) - 1) & ~((powerOf2Alignment) - 1))
#endif

enum CONSTANT_BUFFER_TYPE
{
    CONSTANT_BUFFER_TYPE_GLOBAL = 0,
    CONSTANT_BUFFER_TYPE_MESH,
    CONSTANT_BUFFER_TYPE_SKINNED,
    CONSTANT_BUFFER_TYPE_SPRITE,
    CONSTANT_BUFFER_TYPE_GEOMETRY,
    CONSTANT_BUFFER_TYPE_TERRAIN,
    CONSTANT_BUFFER_TYPE_COUNT
};

struct CONSTANT_BUFFER_PROPERTY
{
    CONSTANT_BUFFER_TYPE type;
    UINT                 size;
};

struct MeshConstants
{
    Matrix world;
    Matrix worldIT;
    DWORD  dummy[32] = {0};
};

static_assert((sizeof(MeshConstants) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

struct SkinnedConstants
{
    Matrix boneTransforms[128]; // bone 개수
};

static_assert((sizeof(SkinnedConstants) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

struct Light : public ILightHandle
{
    Vector3 radiance = Vector3(5.0f); // strength
    float   fallOffStart = 0.0f;
    Vector3 direction = Vector3(0.0f, 0.0f, 1.0f);
    float   fallOffEnd = 20.0f;
    Vector3 position = Vector3(0.0f, 3.0f, -3.0f);
    float   spotPower = 6.0f;

    uint32_t type = LIGHT_OFF;
    float    radius = 0.035f; // 반지름

    float haloRadius = 0.0f;
    float haloStrength = 0.0f;

    Matrix viewProj; // 그림자 렌더링에 필요
    Matrix invProj;  // 그림자 렌더링 디버깅용
};

// register (b0) 사용
struct GlobalConstants
{
    Matrix view;
    Matrix invView;
    Matrix proj;
    Matrix invProj; // 역프로젝션행렬
    Matrix viewProj;
    Matrix invViewProj; // Proj -> World

    Matrix projectionViewProj; // 텍스쳐 투영

    Vector3 eyeWorld;
    float   strengthIBL;

    Light lights[MAX_LIGHTS];
    BOOL  useTextureProjection;

    DWORD dummys[59];
};

static_assert((sizeof(GlobalConstants) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

struct RTGlobalConstancts
{
    Matrix      viewProj;
    Matrix      invView;
    DECOMP_PROJ decompProj;

    Vector3 eyeWorld;

    float Near;
    float Far;
    UINT  maxRadianceRayRecursionDepth;
    UINT  maxShadowRayRecursionDepth;
    UINT  Reserved0;
    Light lights[MAX_LIGHTS];
};

static_assert((sizeof(RTGlobalConstancts) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

enum MATERIAL_USE_FLAGS : UINT
{
    MATERIAL_USE_DEFAULT = 0,
    MATERIAL_USE_ALBEDO_MAP = 0x01,
    MATERIAL_USE_NORMAL_MAP = 0x02,
    MATERIAL_USE_AO_MAP = 0x04,
    MATERIAL_USE_METALLIC_MAP = 0x08,
    MATERIAL_USE_ROUGHNESS_MAP = 0x10,
    MATERIAL_USE_EMISSIVE_MAP = 0x20,
    MATERIAL_USE_HEIGHT_MAP = 0x40,
};

struct MaterialConstants
{
    Vector3 albedo = Vector3(1.0f);
    float   roughnessFactor = 1.0f;
    float   metallicFactor = 1.0f;
    Vector3 emissive = Vector3(0.0f);

    float opacityFactor = 1.0f;
    float reflectionFactor = 0.0f;

    UINT flags = MATERIAL_USE_DEFAULT;

    DWORD dummy[5];
};

static_assert((sizeof(MaterialConstants) % 32) == 0, "Constant Buffer size must be 256-byte aligned");

struct SpriteConstants
{
    Vector2 screenRes;
    Vector2 pos;
    Vector2 scale;
    Vector2 texSize;
    Vector2 texSamplePos;
    Vector2 texSampleSize;
    float   z;
    float   alpha;
    DWORD   dummy[50];
};

static_assert((sizeof(SpriteConstants) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

struct FaceGroupConstants
{
    UINT  materialIndex = 0;
    BOOL  useMaterial = TRUE;
    BOOL  useHeightMap = FALSE;
    DWORD dummy[61];
};
static_assert((sizeof(FaceGroupConstants) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

struct TerrainConstants
{
    UINT  numSlice;
    UINT  numStack;
    float scaleX;
    float scaleY;
    float scaleZ;
    float tessFactor;
    DWORD dummy[58];
};
static_assert((sizeof(TerrainConstants) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

struct RaytracingFaceGroupCB
{
    UINT materialIndex;
    UINT reserved;
};
