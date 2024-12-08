#include "pch.h"

#include "GameObject.h"

#include "World.h"

void World::Initialize(GameObject **pObjectArray, int objectCount)
{ 
	m_pTree = new KDTree;

	Bounds *pBoundArray = new Bounds[objectCount];
	for (int i = 0; i < objectCount; i++)
	{
        pBoundArray[i] = pObjectArray[i]->GetBounds();
	}

    m_pTree->Initialize(pBoundArray, reinterpret_cast<void**>(pObjectArray), objectCount);

	if (pBoundArray)
	{
        delete[] pBoundArray;
        pBoundArray = nullptr;
	}
}
