#include "pch.h"
#include "Collider.h"

Collider *Collider::CreateBoxCollider(const Vector3 &halfExtents) 
{ 
    Collider *pNew = new Collider;
    pNew->m_pShape = new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
    return pNew;
}

Collider *Collider::CreateSphereCollider(const float radius)
{
    Collider *pNew = new Collider;
    pNew->m_pShape = new btSphereShape(btScalar(radius));
    return pNew;
}

Collider *Collider::CreateCapsuleCollider(const float radius, const float height) 
{ 
    Collider *pNew = new Collider;
    pNew->m_pShape = new btCapsuleShape(btScalar(radius), btScalar(height));
    return pNew;
}

Collider *Collider::CreateConvexCollider(const Vector3 *points, const int numPoints) 
{ 
    Collider *pNew = new Collider;
    pNew->m_pShape = new btConvexHullShape((btScalar*)points, numPoints, sizeof(Vector3));
    return pNew;
}
