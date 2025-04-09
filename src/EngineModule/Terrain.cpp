#include "pch.h"

#include "GameManager.h"

#include "Terrain.h"

void Terrain::Cleanup() 
{ 
    if (m_pRenderTerrain)
    {
        m_pRenderTerrain->Release();
        m_pRenderTerrain = nullptr;
    }
}

BOOL Terrain::Initialize(const Material *pMaterial, const Vector3 &scale, const int numSlice, const int numStack)
{
    IRenderer       *pRenderer = g_pGame->GetRenderer();
    IPhysicsManager *pPhysics = g_pGame->GetPhysicsManager();
    m_pRenderTerrain = pRenderer->CreateTerrain(pMaterial, &scale, numSlice, numStack);

    WCHAR filename[MAX_PATH] = {L'\0'};
    wcscpy_s(filename, pMaterial->basePath);
    wcscat_s(filename, pMaterial->heightTextureName);

    int   width, height;
    BYTE *pImage;
    BOOL result = CreateHeightMapFromFile(filename, &pImage, &width, &height);
    DASSERT(result);

    m_pCollider = pPhysics->CreateHeightFieldTerrain(pImage, width, height, scale, 0.0f, scale.y);
    m_terrainScale = scale;
    m_pImage = pImage;
    return TRUE;
}

void Terrain::Render() { g_pGame->GetRenderer()->RenderTerrain(m_pRenderTerrain, &m_terrainScale, false); }

Terrain::~Terrain() { Cleanup(); }
