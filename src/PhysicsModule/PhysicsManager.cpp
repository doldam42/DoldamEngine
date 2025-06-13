#include "pch.h"

#include "BroadPhase.h"

#include "ConvexShape.h"
#include "Shape.h"

#include "Collider.h"
#include "RigidBody.h"

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
            goto lb_success;
        }
    }
    else if (typeA == SHAPE_TYPE_SPHERE && typeB == SHAPE_TYPE_BOX)
    {
        const SphereShape *pSphere = (const SphereShape *)pA->pShape;
        const BoxShape    *pBox = (const BoxShape *)pB->pShape;
        if (SphereBoxStatic(pSphere->Radius, posA, pBox->HalfExtent, rotB, posB, &contactPointA, &contactPointB))
        {
            goto lb_success;
        }
    }
    else if (typeA == SHAPE_TYPE_SPHERE && typeB == SHAPE_TYPE_ELLIPSOID)
    {
        const SphereShape    *pSphere = (const SphereShape *)pA->pShape;
        const EllipsoidShape *pEllipse = (const EllipsoidShape *)pB->pShape;
        if (EllipseEllipseStatic(pSphere->Radius, pEllipse->MajorRadius, pSphere->Radius, pEllipse->MinorRadius, posA,
                                 posB, &contactPointA, &contactPointB))
        {
            goto lb_success;
        }
    }
    else if (typeA == SHAPE_TYPE_BOX && typeB == SHAPE_TYPE_BOX)
    {
        const BoxShape *pBoxA = (const BoxShape *)pA->pShape;
        const BoxShape *pBoxB = (const BoxShape *)pB->pShape;

        if (BoxBoxStatic(pBoxA->HalfExtent, pBoxB->HalfExtent, pA->Rotation, pB->Rotation, posA, posB, &contactPointA,
                         &contactPointB))
        {
            goto lb_success;
        }
    }
    else if (typeA == SHAPE_TYPE_BOX && typeB == SHAPE_TYPE_ELLIPSOID)
    {
        const BoxShape       *pBox = (const BoxShape *)pA->pShape;
        const EllipsoidShape *pEllipse = (const EllipsoidShape *)pB->pShape;
        if (BoxEllipseStatic(pBox->HalfExtent, rotA, posA, pEllipse->MajorRadius, pEllipse->MinorRadius, posB,
                             &contactPointA, &contactPointB))
        {
            goto lb_success;
        }
    }
    else if (typeA == SHAPE_TYPE_ELLIPSOID && typeB == SHAPE_TYPE_ELLIPSOID)
    {
        const EllipsoidShape *pEllipseA = (const EllipsoidShape *)pA;
        const EllipsoidShape *pEllipseB = (const EllipsoidShape *)pB;
        if (EllipseEllipseStatic(pEllipseA->MajorRadius, pEllipseB->MajorRadius, pEllipseA->MinorRadius,
                                 pEllipseB->MinorRadius, posA, posB, &contactPointA, &contactPointB))
        {
            goto lb_success;
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

            contactPointA = ptOnA - normal * bias;
            contactPointB = ptOnB + normal * bias;

            goto lb_success;
        }

        // there was no collision, but we still want the contact data, so get it
        GJK_ClosestPoints(pA, pB, &ptOnA, &ptOnB);
        contact.contactPointAWorldSpace = ptOnA;
        contact.contactPointBWorldSpace = ptOnB;

        *pOutContact = contact;
    }

    return FALSE;

lb_success:
    pOutContact->pA = pA;
    pOutContact->pB = pB;
    pOutContact->contactPointAWorldSpace = contactPointA;
    pOutContact->contactPointBWorldSpace = contactPointB;
    pOutContact->normal = contactPointA - contactPointB;
    pOutContact->normal.Normalize();
    return TRUE;
}

BOOL PhysicsManager::Intersect(RigidBody *pA, RigidBody *pB, const float dt, Contact *pOutContact)
{
    if (!pA || !pB)
        return FALSE;

    pOutContact->timeOfImpact = 0.0f;

    Collider *pColliderA = pA->GetCollider();
    Collider *pColliderB = pB->GetCollider();

    SHAPE_TYPE typeA = pColliderA->pShape->GetType();
    SHAPE_TYPE typeB = pColliderB->pShape->GetType();

    // 정렬해서 중복 제거
    if (typeA > typeB)
    {
        std::swap(typeA, typeB);
        std::swap(pA, pB);
        std::swap(pColliderA, pColliderB);
    }

    const Vector3 &posA = pColliderA->Position;
    const Vector3 &posB = pColliderB->Position;

    const Quaternion &rotA = pColliderA->Rotation;
    const Quaternion &rotB = pColliderB->Rotation;

    const Vector3 velA = pA->GetVelocity();
    const Vector3 velB = pB->GetVelocity();

    float   timeOfImpact;
    Vector3 contactPointA;
    Vector3 contactPointB;

    if (typeA == SHAPE_TYPE_SPHERE && typeB == SHAPE_TYPE_SPHERE)
    {
        const SphereShape *pSphereA = (const SphereShape *)pColliderA->pShape;
        const SphereShape *pSphereB = (const SphereShape *)pColliderB->pShape;

        if (SphereSphereDynamic(pSphereA->Radius, pSphereB->Radius, posA, posB, velA, velB, dt, &contactPointA,
                                &contactPointB, &timeOfImpact))
        {
            goto lb_success;
        }
    }
    else if (typeA == SHAPE_TYPE_SPHERE && typeB == SHAPE_TYPE_ELLIPSOID)
    {
        Vector3               normal;
        const SphereShape    *pSphere = (const SphereShape *)pColliderA->pShape;
        const EllipsoidShape *pEllipse = (const EllipsoidShape *)pColliderB->pShape;
        if (EllipseEllipseDynamic(pSphere->Radius, pEllipse->MajorRadius, pSphere->Radius, pEllipse->MinorRadius, posA,
            posB, velA, velB, dt, &normal, &timeOfImpact))
        {
            goto lb_success;
        }
    }
    else if (typeA == SHAPE_TYPE_BOX && typeB == SHAPE_TYPE_BOX)
    {
        const BoxShape *pBoxA = (const BoxShape *)pColliderA->pShape;
        const BoxShape *pBoxB = (const BoxShape *)pColliderB->pShape;

        if (BoxBoxDynamic(pBoxA->HalfExtent, pBoxB->HalfExtent, rotA, rotB, posA, posB, velA, velB, &contactPointA,
            &contactPointB, &timeOfImpact))
        {
            goto lb_success;
        }
    }
    else if (typeA == SHAPE_TYPE_ELLIPSOID && typeB == SHAPE_TYPE_ELLIPSOID)
    {
        const EllipsoidShape *pEllipseA = (const EllipsoidShape *)pColliderA;
        const EllipsoidShape *pEllipseB = (const EllipsoidShape *)pColliderB;
        if (EllipseEllipseDynamic(pEllipseA->MajorRadius, pEllipseB->MajorRadius, pEllipseA->MinorRadius,
                                 pEllipseB->MinorRadius, posA, posB, velA, velB, dt, &contactPointA, &contactPointB))
        {
            goto lb_success;
        }
    }
    else
    {
        Contact     contact;
        Vector3     ptOnA, ptOnB;
        const float bias = 0.001f;
        if (GJK_DoesIntersect(pColliderA, pColliderB, bias, &ptOnA, &ptOnB))
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
        GJK_ClosestPoints(pColliderA, pColliderB, &ptOnA, &ptOnB);
        contact.contactPointAWorldSpace = ptOnA;
        contact.contactPointBWorldSpace = ptOnB;

        *pOutContact = contact;
    }

    return FALSE;

lb_success:
    pOutContact->pA = pColliderA;
    pOutContact->pB = pColliderB;

    pA->Update(timeOfImpact);
    pB->Update(timeOfImpact);

    pOutContact->contactPointAWorldSpace = contactPointA;
    pOutContact->contactPointBWorldSpace = contactPointB;

    // Convert world space contacts to local space
    /*pOutContact->contactPointALocalSpace = pA->WorldSpaceToLocalSpace(contactPointAWorldSpace);
    pOutContact->contactPointBLocalSpace = pB->WorldSpaceToLocalSpace(contactPointBWorldSpace);*/

    pOutContact->normal = posA - posB;
    pOutContact->timeOfImpact = timeOfImpact;

    // Unwind time step
    pA->Update(-timeOfImpact);
    pB->Update(-timeOfImpact);

    //// Calculate the separation distance
    // Vector3 ab = posB - posA;
    // float   r = ab.Length() - (pSphereA->Radius + pSphereB->Radius);

    // pOutContact->separationDistance = r;

    return TRUE;
}

void PhysicsManager::ApplyGravityImpulseAll(float dt)
{
    SORT_LINK *pCur = m_pRigidBodyLinkHead;
    while (pCur)
    {
        RigidBody *pBody = (RigidBody *)pCur->pItem;
        pBody->ApplyGravityImpulse(dt);
        pCur = pCur->pNext;
    }
}

int CompareContacts(const void *p1, const void *p2)
{
    Contact a = *(Contact *)p1;
    Contact b = *(Contact *)p2;

    if (a.timeOfImpact < b.timeOfImpact)
    {
        return -1;
    }

    if (a.timeOfImpact == b.timeOfImpact)
    {
        return 0;
    }

    return 1;
}

void PhysicsManager::ResolveContactsAll(float dt)
{
    if (m_contactCount > 1)
    {
        qsort(m_contacts, m_contactCount, sizeof(Contact), CompareContacts);
    }

    float accumulatedTime = 0.0f;
    for (UINT i = 0; i < m_contactCount; i++)
    {
        Contact    &contact = m_contacts[i];
        const float dt_sec = contact.timeOfImpact - accumulatedTime;

        // position Update
        SORT_LINK *pCur = m_pRigidBodyLinkHead;
        while (pCur)
        {
            RigidBody *pBody = (RigidBody *)pCur->pItem;
            pBody->Update(dt_sec);
            pCur = pCur->pNext;
        }
        ResolveContact(contact);
        accumulatedTime += dt_sec;
    }

    // Update the positions for the rest of this frame's time
    const float timeRemaining = dt - accumulatedTime;
    if (timeRemaining > 0.0f)
    {
        SORT_LINK *pCur = m_pRigidBodyLinkHead;
        while (pCur)
        {
            RigidBody *pBody = (RigidBody *)pCur->pItem;
            pBody->Update(timeRemaining);
            pCur = pCur->pNext;
        }
    }
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

ICollider *PhysicsManager::CreateConvexCollider(IGameObject *pObj, const Vector3 *points, const int numPoints)
{
    Collider *pNew = new Collider;

    m_pColliders[m_colliderCount] = pNew;

    pNew->pShape = new ConvexShape(points, numPoints);
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

IRigidBody *PhysicsManager::CreateRigidBody(ICollider *pCollider, const Vector3 &pos, float mass, float elasticity,
                                            float friction, BOOL useGravity)
{
    RigidBody *pNew = new RigidBody;
    pNew->Initialize((Collider *)pCollider, mass, elasticity, friction, useGravity);

    LinkToLinkedList(&m_pRigidBodyLinkHead, &m_pRigidBodyLinkTail, &pNew->m_linkInPhysics);

    return pNew;
}

void PhysicsManager::DeleteRigidBody(IRigidBody *pDel)
{
    RigidBody *pBody = (RigidBody *)pDel;

    UnLinkFromLinkedList(&m_pRigidBodyLinkHead, &m_pRigidBodyLinkTail, &pBody->m_linkInPhysics);

    delete pBody;
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
    ApplyGravityImpulseAll(dt);

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

            DASSERT(dataA.PairCount < MAX_PAIR_PER_COLLIDER && dataB.PairCount < MAX_PAIR_PER_COLLIDER);

            dataA.PairIndices[dataA.PairCount] = pB->ID;
            dataA.ContactIndices[dataA.PairCount] = m_contactCount;
            dataA.PairCount++;

            dataB.PairIndices[dataB.PairCount] = pA->ID;
            dataB.ContactIndices[dataB.PairCount] = m_contactCount;
            dataB.PairCount++;

            m_contacts[m_contactCount++] = contact;
            pA->IsCollide = TRUE;
            pB->IsCollide = TRUE;
        }
    }

    return m_contactCount > 0;
}

void PhysicsManager::EndCollision() { m_contactCount = 0; }
