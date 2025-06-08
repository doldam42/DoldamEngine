#include "pch.h"

#include "BroadPhase.h"

#include "ColliderBase.h"
#include "Shape.h"

#include "GJK.h"

#include "PhysicsManager.h"

PhysicsManager *g_pPhysics = nullptr;

HRESULT __stdcall PhysicsManager::QueryInterface(REFIID riid, void **ppvObject) { return E_NOTIMPL; }

ULONG __stdcall PhysicsManager::AddRef(void)
{
    m_refCount++;
    return m_refCount;
}

ULONG __stdcall PhysicsManager::Release(void)
{
    ULONG ref_count = --m_refCount;
    if (!m_refCount)
        delete this;

    return ref_count;
}

BOOL PhysicsManager::Intersect(Collider *pA, Collider *pB, Contact *pOutContact)
{
    SHAPE_TYPE typeA = pA->pShape->GetType();
    SHAPE_TYPE typeB = pB->pShape->GetType();

    // 정렬해서 중복 제거
    if (typeA > typeB)
    {
        std::swap(typeA, typeB);
        std::swap(pA, pB);
    }

    const Vector3 &posA = pA->Position;
    const Vector3 &posB = pB->Position;

    const Quaternion &rotA = pA->Rotation;
    const Quaternion &rotB = pB->Rotation;

    Vector3 contactPointA;
    Vector3 contactPointB;
    if (typeA == SHAPE_TYPE_SPHERE && typeB == SHAPE_TYPE_SPHERE)
    {
        const SphereShape *pSphereA = (const SphereShape *)pA->pShape;
        const SphereShape *pSphereB = (const SphereShape *)pB->pShape;
       
        if (SphereSphereStatic(pSphereA->Radius, pSphereB->Radius, posA, posB, &contactPointA, &contactPointB))
        {
            pOutContact->pA = pA;
            pOutContact->pB = pB;
            pOutContact->contactPointAWorldSpace = contactPointA;
            pOutContact->contactPointBWorldSpace = contactPointB;
            pOutContact->normal = contactPointA - contactPointB;
            pOutContact->normal.Normalize();

            return TRUE;
        }
    }
    else if (typeA == SHAPE_TYPE_SPHERE && typeB == SHAPE_TYPE_BOX)
    {
        const SphereShape *pSphere = (const SphereShape *)pA->pShape;
        const BoxShape    *pBox = (const BoxShape *)pB->pShape;
        if (SphereBoxStatic(pSphere->Radius, posA, pBox->HalfExtent, rotB, posB, &contactPointA, &contactPointB))
        {
            pOutContact->pA = pA;
            pOutContact->pB = pB;
            pOutContact->contactPointAWorldSpace = contactPointA;
            pOutContact->contactPointBWorldSpace = contactPointB;
            pOutContact->normal = contactPointA - contactPointB;
            pOutContact->normal.Normalize();
            return TRUE;
        }
    }
    else if (typeA == SHAPE_TYPE_SPHERE && typeB == SHAPE_TYPE_ELLIPSOID)
    {
        const SphereShape    *pSphere = (const SphereShape *)pA->pShape;
        const EllipsoidShape *pEllipse = (const EllipsoidShape *)pB->pShape;
        if (EllipseEllipseStatic(pSphere->Radius, pEllipse->MajorRadius, pSphere->Radius, pEllipse->MinorRadius, posA,
                                 posB, &contactPointA, &contactPointB))
        {
            pOutContact->pA = pA;
            pOutContact->pB = pB;
            pOutContact->contactPointAWorldSpace = contactPointA;
            pOutContact->contactPointBWorldSpace = contactPointB;
            pOutContact->normal = contactPointA - contactPointB;
            pOutContact->normal.Normalize();
            return TRUE;
        }
    }
    else if (typeA == SHAPE_TYPE_BOX && typeB == SHAPE_TYPE_BOX)
    {
        const BoxShape *pBoxA = (const BoxShape *)pA->pShape;
        const BoxShape *pBoxB = (const BoxShape *)pB->pShape;

        if (BoxBoxStatic(pBoxA->HalfExtent, pBoxB->HalfExtent, pA->Rotation, pB->Rotation, posA, posB, &contactPointA,
                         &contactPointB))
        {
            pOutContact->pA = pA;
            pOutContact->pB = pB;
            pOutContact->contactPointAWorldSpace = contactPointA;
            pOutContact->contactPointBWorldSpace = contactPointB;
            pOutContact->normal = contactPointA - contactPointB;
            pOutContact->normal.Normalize();
            return TRUE;
        }
    }
    else if (typeA == SHAPE_TYPE_BOX && typeB == SHAPE_TYPE_ELLIPSOID)
    {
        const BoxShape       *pBox = (const BoxShape *)pA->pShape;
        const EllipsoidShape *pEllipse = (const EllipsoidShape *)pB->pShape;
        if (BoxEllipseStatic(pBox->HalfExtent, rotA, posA, pEllipse->MajorRadius, pEllipse->MinorRadius, posB,
                             &contactPointA, &contactPointB))
        {
            pOutContact->pA = pA;
            pOutContact->pB = pB;
            pOutContact->contactPointAWorldSpace = contactPointA;
            pOutContact->contactPointBWorldSpace = contactPointB;
            pOutContact->normal = contactPointA - contactPointB;
            pOutContact->normal.Normalize();
            return TRUE;
        }
    }
    else if (typeA == SHAPE_TYPE_ELLIPSOID && typeB == SHAPE_TYPE_ELLIPSOID)
    {
        const EllipsoidShape *pEllipseA = (const EllipsoidShape *)pA;
        const EllipsoidShape *pEllipseB = (const EllipsoidShape *)pB;
        if (EllipseEllipseStatic(pEllipseA->MajorRadius, pEllipseB->MajorRadius, pEllipseA->MinorRadius,
                                 pEllipseB->MinorRadius, posA, posB, &contactPointA, &contactPointB))
        {
            pOutContact->pA = pA;
            pOutContact->pB = pB;
            pOutContact->contactPointAWorldSpace = contactPointA;
            pOutContact->contactPointBWorldSpace = contactPointB;
            pOutContact->normal = contactPointA - contactPointB;
            pOutContact->normal.Normalize();
            return TRUE;
        }
    }
    else
    {
        Contact     contact;
        Vector3     ptOnA, ptOnB;
        const float bias = 0.001f;
        if (GJK_DoesIntersect(pA, pB, bias, &ptOnA, &ptOnB))
        {
            Vector3 normal = ptOnB - ptOnA;
            normal.Normalize();

            ptOnA -= normal * bias;
            ptOnB += normal * bias;

            contact.normal = normal;
            contact.contactPointAWorldSpace = ptOnA;
            contact.contactPointBWorldSpace = ptOnB;

            *pOutContact = contact;
            return TRUE;
        }

        // there was no collision, but we still want the contact data, so get it
        GJK_ClosestPoints(pA, pB, &ptOnA, &ptOnB);
        contact.contactPointAWorldSpace = ptOnA;
        contact.contactPointBWorldSpace = ptOnB;

        *pOutContact = contact;
    }

    return FALSE;
}

BOOL PhysicsManager::Initialize()
{
    m_pBroadPhase = new BroadPhase;
    m_pBroadPhase->Initialize(MAX_BODY_COUNT, Vector3::One);

    g_pPhysics = this;

    return TRUE;
}

ICollider *PhysicsManager::CreateSphereCollider(IGameObject *pObj, const float radius)
{
    Collider *pNew = new Collider;
    
    m_pColliders[m_colliderCount] = pNew;

    pNew->pShape = new SphereShape(radius);
    pNew->Position = pObj->GetPosition();
    pNew->Rotation = pObj->GetRotation();
    pNew->pObj = pObj;
    pNew->ID = m_colliderCount;
    m_colliderCount++;
    return pNew;
}

ICollider *PhysicsManager::CreateBoxCollider(IGameObject *pObj, const Vector3 &halfExtents)
{
    Collider *pNew = new Collider;

    m_pColliders[m_colliderCount] = pNew;

    pNew->pShape = new BoxShape(halfExtents);
    pNew->Position = pObj->GetPosition();
    pNew->Rotation = pObj->GetRotation();
    pNew->pObj = pObj;
    pNew->ID = m_colliderCount;
    m_colliderCount++;
    return pNew;
}

ICollider *PhysicsManager::CreateEllipsoidCollider(IGameObject *pObj, const float majorRadius, const float minorRadius)
{
    Collider *pNew = new Collider;

    m_pColliders[m_colliderCount] = pNew;

    pNew->pShape = new EllipsoidShape(majorRadius, minorRadius);
    pNew->Position = pObj->GetPosition();
    pNew->Rotation = pObj->GetRotation();
    pNew->pObj = pObj;
    pNew->ID = m_colliderCount;
    m_colliderCount++;
    return pNew;
}

void PhysicsManager::DeleteCollider(ICollider *pDel)
{
    Collider *pCol = (Collider *)pDel;

    UINT idx = pCol->ID;
    m_pColliders[idx] = m_pColliders[m_colliderCount - 1];
    m_colliderCount--;
    m_pColliders[idx]->ID = idx;
    delete pDel;
}

BOOL PhysicsManager::Raycast(const Ray &ray, Vector3 *pOutNormal, float *tHit, ICollider **pCollider)
{
    Vector3    normal;
    float      tMin = FLT_MAX;
    ICollider *pResult = nullptr;
    for (int i = 0; i < m_colliderCount; i++)
    {
        Collider *pCollider = m_pColliders[i];
        Vector3   n;
        float     t;
        if (pCollider->RayTest(ray, &n, &t) && t < tMin)
        {
            normal = n;
            tMin = t;
            pResult = pCollider;
        }
    }

    if (tMin < ray.tmax)
    {
        *pOutNormal = normal;
        *pCollider = pResult;
        *tHit = tMin;
        return TRUE;
    }
    return FALSE;
}

void PhysicsManager::BeginCollision(float dt)
{
    for (int i = 0; i < m_colliderCount; i++)
    {
        Collider *pCollider = m_pColliders[i];
        pCollider->IsPrevCollide = pCollider->IsCollide;
        pCollider->IsCollide = FALSE;
    }

    m_pBroadPhase->Build(m_pColliders, m_colliderCount, dt);
}

BOOL PhysicsManager::CollisionTestAll(float dt)
{
    for (int i = 0; i < m_colliderCount; i++)
    {
        m_colliderData[i].PairCount = 0;
    }

    ZeroMemory(m_collisionPairs, sizeof(CollisionPair) * MAX_COLLISION_CANDIDATE_COUNT);
    UINT numCandidate = m_pBroadPhase->QueryCollisionPairs(m_collisionPairs, MAX_COLLISION_CANDIDATE_COUNT);
    for (UINT i = 0; i < numCandidate; i++)
    {
        const CollisionPair &pair = m_collisionPairs[i];

        Collider *pA = m_pColliders[pair.a];
        Collider *pB = m_pColliders[pair.b];

        Contact contact;
        if (pA->IsActive && pB->IsActive && Intersect(pA, pB, &contact))
        {
            ColliderData &dataA = m_colliderData[pA->ID];
            ColliderData &dataB = m_colliderData[pB->ID];
            if (dataA.PairCount < MAX_PAIR_PER_COLLIDER)
            {
                dataA.PairIndices[dataA.PairCount] = pB->ID;
                dataA.ContactIndices[dataA.PairCount] = m_contactCount;
                dataA.PairCount++;
            }
            if (dataB.PairCount < MAX_PAIR_PER_COLLIDER)
            {
                dataB.PairIndices[dataB.PairCount] = pA->ID;
                dataB.ContactIndices[dataB.PairCount] = m_contactCount;
                dataB.PairCount++;
            }

            m_contacts[m_contactCount++] = contact;
            pA->IsCollide = TRUE;
            pB->IsCollide = TRUE;
        }
    }

    return m_contactCount > 0;
}

void PhysicsManager::EndCollision() { m_contactCount = 0; }
