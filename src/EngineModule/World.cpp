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

void World::BeginCreateWorld(UINT maxObjectCount)
{ 
    /*m_pTree = new BVH(maxObjectCount);*/
    m_pTree = new KDTree(maxObjectCount);
}

void World::InsertObject(GameObject *pObject) { m_pTree->InsertObject(pObject); }

void World::EndCreateWorld() { m_pTree->Build(); }

bool World::Intersect(const Ray &ray, RayHit *pOutHit) 
{
    float tHit;
    GameObject *pOut = nullptr;
    if (m_pTree->IntersectP(ray, &tHit, reinterpret_cast<IBoundedObject**>(&pOut)))
    {
        pOutHit->tHit = tHit;
        pOutHit->pHitted = pOut;
        return TRUE;
    }
    return FALSE;
}

World::~World() { Cleanup(); }
