#include "pch.h"

#include "Intersect.h"

static BOOL SphereSphereStatic(const Sphere &a, const Sphere &b, const Vector3 &posA, const Vector3 &posB,
                        Vector3 *pOutContactPointA, Vector3 *pOutContactPointB, Vector3 *pOutNormal)
{
    Vector3 ab = posB - posA;

    const float radiusAB = a.Radius + b.Radius;
    const float lengthSquared = ab.LengthSquared();
    if (lengthSquared > (radiusAB * radiusAB))
    {
        return FALSE;
    }

    Vector3 normal = ab;
    normal.Normalize();

    *pOutContactPointA = posA + normal * a.Radius;
    *pOutContactPointB = posB - normal * b.Radius;

    *pOutNormal = normal;

    return TRUE;
}

BOOL Intersect(PhysicsComponent *pA, PhysicsComponent *pB, Contact *pOutContact)
{
    if (!pA || !pB)
        return FALSE;

    pOutContact->pA = pA;
    pOutContact->pB = pB;

    Vector3 posA = pA->GetPosition();
    Vector3 posB = pB->GetPosition();

    if (pA->m_pShape->GetType() == SHAPE_TYPE_SPHERE && pB->m_pShape->GetType() == SHAPE_TYPE_SPHERE)
    {
        Sphere *pSphereA = (Sphere *)pA->m_pShape;
        Sphere *pSphereB = (Sphere *)pB->m_pShape;
        
        Vector3 contactPointA;
        Vector3 contactPointB;
        Vector3 normal;
        if (SphereSphereStatic(*pSphereA, *pSphereB, posA, posB, &contactPointA, &contactPointB, &normal))
        {
            pOutContact->contactPointA = contactPointA;
            pOutContact->contactPointB = contactPointB;
            pOutContact->normal = normal;

            return TRUE;
        }
        return FALSE;
    }
}
