#include "pch.h"

#include "Collider.h"
#include "RigidBody.h"

#include "BroadPhase.h"


/*
====================================================
CompareSAP
====================================================
*/
static int CompareSAP(const void *a, const void *b)
{
    const PsuedoBody *ea = (const PsuedoBody *)a;
    const PsuedoBody *eb = (const PsuedoBody *)b;

    if (ea->value < eb->value)
    {
        return -1;
    }
    return 1;
}


static bool pred(const PsuedoBody &a, const float value) { return a.value < value; }

/*
====================================================
SortBodiesBounds
====================================================
*/
void BroadPhase::SortBodiesBounds(const Collider *const *bodies, const int num, PsuedoBody *sortedArray,
                                  const float dt_sec)
{
    for (int i = 0; i < num; i++)
    {
        const Collider *collider = bodies[i];
        Bounds          bounds = collider->GetBounds();

        if (collider->pBody)
        {
            // Expand the bounds by the linear velocity
            bounds.Expand(bounds.mins + collider->pBody->GetVelocity() * dt_sec);
            bounds.Expand(bounds.maxs + collider->pBody->GetVelocity() * dt_sec);
        }
        
        const float epsilon = 0.01f;
        bounds.Expand(bounds.mins - Vector3(epsilon));
        bounds.Expand(bounds.maxs + Vector3(epsilon));

        sortedArray[i * 2 + 0].id = i;
        sortedArray[i * 2 + 0].value = m_axisSAP.Dot(bounds.mins);
        sortedArray[i * 2 + 0].isMin = true;

        sortedArray[i * 2 + 1].id = i;
        sortedArray[i * 2 + 1].value = m_axisSAP.Dot(bounds.maxs);
        sortedArray[i * 2 + 1].isMin = false;
    }

    qsort(sortedArray, num * 2, sizeof(PsuedoBody), CompareSAP);
}


BOOL BroadPhase::Initialize(const UINT maxBodyCount, Vector3 axisSAP)
{
    m_pPsudoBodies = new PsuedoBody[maxBodyCount * 2];
    ZeroMemory(m_pPsudoBodies, sizeof(PsuedoBody) * maxBodyCount * 2);

    m_axisSAP = axisSAP;
    m_axisSAP.Normalize();

    m_bodyCount = 0;
    m_maxBodyCount = maxBodyCount;

    return TRUE;
}

void BroadPhase::Build(const Collider *const *bodies, const int num, const float dt_sec)
{
    m_bodyCount = num;
    SortBodiesBounds(bodies, num, m_pPsudoBodies, dt_sec);
}

UINT BroadPhase::QueryCollisionPairs(CollisionPair *pCollisionPairs, UINT maxCollision)
{
    UINT numCollision = 0;
    // Now that the bodies are sorted, build the collision pairs
    for (int i = 0; i < m_bodyCount * 2; i++)
    {
        const PsuedoBody &a = m_pPsudoBodies[i];
        if (!a.isMin)
        {
            continue;
        }

        CollisionPair pair;
        pair.a = a.id;

        for (int j = i + 1; j < m_bodyCount * 2; j++)
        {
            const PsuedoBody &b = m_pPsudoBodies[j];
            // if we've hit the end of the a element, then we're done creating pairs with a
            if (b.id == a.id)
            {
                break;
            }

            if (!b.isMin)
            {
                continue;
            }

            pair.b = b.id;

            pCollisionPairs[numCollision] = pair;
            numCollision++;

            if (numCollision == maxCollision)
            {
                __debugbreak();
                return numCollision;
            }
        }
    }

    return numCollision;
}

void BroadPhase::Reset() { m_bodyCount = 0; }