#pragma once
class Terrain
{
    BYTE *m_pImage = nullptr;
    UINT  m_imgWidth = 0;
    UINT  m_imgHeight = 0;

    IRenderTerrain              *m_pRenderTerrain = nullptr;

    Vector3 m_terrainScale;

    void Cleanup();

  public:
    BOOL Initialize(const Material *pMaterial, const Vector3 &scale, const int numSlice = 0, const int numStack = 0);

    void Render();

    Terrain() = default;
    ~Terrain();
};
