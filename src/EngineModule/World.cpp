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

void World::Initialize() {}

void World::BeginCreateWorld(UINT maxObjectCount) { m_pTree = new KDTree(maxObjectCount); }

void World::InsertObject(GameObject *pObject) { m_pTree->InsertObject(pObject); }

void World::EndCreateWorld() { m_pTree->Build(); }

bool World::Intersect(const Ray &ray) { return m_pTree->IntersectP(ray); }

World::~World() { Cleanup(); }
