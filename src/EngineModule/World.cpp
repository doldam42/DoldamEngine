#include "pch.h"

#include "GameObject.h"

#include "World.h"

void World::Cleanup() 
{ 
    if (m_pTree)
    {
        delete m_pTree;
        m_pTree = nullptr;
    }
}

void World::Initialize() { m_pTree = new KDTree; }

void World::BeginCreateWorld(UINT maxObjectCount) { m_pTree->BeginCreateTree(maxObjectCount); }

void World::InsertObject(GameObject *pObject)
{
    Bounds b = pObject->GetBounds();
    m_pTree->InsertObject(&b, &pObject->m_LinkInWorld);
}

void World::EndCreateWorld()
{
    m_pTree->EndCreateTree();
#ifdef _DEBUG
    // m_pTree->DebugPrintTree();
#endif // _DEBUG
}

World::~World() { Cleanup(); }
