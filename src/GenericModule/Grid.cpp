#include "pch.h"
#include "Grid.h"

size_t Grid::GetLocationIndex(const Vector3 &objectCenter)
{
    Vector3 square = (objectCenter - m_minCorner) * m_invCellSize;

    UINT x = square.x;
    UINT z = square.z;

    assert(x >= 0 && z >= 0);

    return x + m_numSliceX * z;
}

BOOL Grid::Initialize(const Vector3 &center, const Vector3 &extents, float cellSize)
{
    m_center = m_center;
    m_extents = extents;
    m_minCorner = center - extents;
    m_invCellSize = 1.0f / cellSize;

    const Vector3 numSlice = extents * (2.0f * m_invCellSize);
    m_numSliceX = numSlice.x;
    m_numSliceZ = numSlice.z;

    m_pObjectSets = new std::set<IBoundedObject *>[m_numSliceX * m_numSliceZ];
    m_pActiveSets = new std::set<IBoundedObject *>[m_numSliceX * m_numSliceZ];

    return TRUE;
}

BOOL Grid::Insert(IBoundedObject *pObj)
{
    const Bounds &b = pObj->GetBounds();
    const Vector3 center = b.Center();
    const UINT    idx = GetLocationIndex(center);

    std::set<IBoundedObject *> pLoc = m_pObjectSets[idx];
    if (pLoc.size() == 1)
    {
        m_pActiveSets[m_activeSetCount] = pLoc;
        m_activeSetCount++;
    }
    pLoc.insert(pObj);

    return TRUE;
}

BOOL Grid::Remove(IBoundedObject *pObj)
{
    const Bounds &b = pObj->GetBounds();
    const Vector3 center = b.Center();
    const UINT    idx = GetLocationIndex(center);

    std::set<IBoundedObject *> pLoc = m_pObjectSets[idx];
    if (pLoc.size() == 2)
    {
        for (int i = 0; i < m_activeSetCount; i++)
        {
            if (m_pActiveSets[i] == pLoc)
            {
                m_pActiveSets[i] = m_pActiveSets[m_activeSetCount - 1];
                m_activeSetCount--;
                break;
            }
        }
    }
    pLoc.erase(pObj);
    return TRUE;
}
