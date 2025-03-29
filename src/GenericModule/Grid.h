#pragma once
class Grid
{
    std::set<void*> *m_pObjectSets;
    std::set<void*> *m_pActiveSets;

    Vector3 m_minCorner;
    Vector3 m_center;
    Vector3 m_extents;

    UINT  m_numSliceX = 0;
    UINT  m_numSliceZ = 0;
    UINT  m_activeSetCount = 0;
    float m_invCellSize = 0.0f;

    size_t GetLocationIndex(const Vector3 &objectCenter);

  public:
    BOOL Initialize(const Vector3 &center, const Vector3 &extents, float cellSize);

    BOOL Insert(const Bounds &b, void *pObj);
    BOOL Remove(const Bounds &b, void *pObj);
};
