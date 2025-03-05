#include "pch.h"
#include "BroadPhase.h"

struct PsuedoBody
{
    int   id;
    float value;
    bool  isMin;
};

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

/*
====================================================
SortBodiesBounds
====================================================
*/
static void SortBodiesBounds(const RigidBody *const *bodies, const int num, PsuedoBody *sortedArray, const float dt_sec)
{
    Vector3 axis = Vector3(1, 1, 1);
    axis.Normalize();

    for (int i = 0; i < num; i++)
    {
        const RigidBody *body = bodies[i];
        Bounds           bounds = body->GetBounds();

        // Expand the bounds by the linear velocity
        bounds.Expand(bounds.mins + body->m_linearVelocity * dt_sec);
        bounds.Expand(bounds.maxs + body->m_linearVelocity * dt_sec);

        const float epsilon = 0.01f;
        bounds.Expand(bounds.mins + Vector3(-1, -1, -1) * epsilon);
        bounds.Expand(bounds.maxs + Vector3(1, 1, 1) * epsilon);

        sortedArray[i * 2 + 0].id = i;
        sortedArray[i * 2 + 0].value = axis.Dot(bounds.mins);
        sortedArray[i * 2 + 0].isMin = true;

        sortedArray[i * 2 + 1].id = i;
        sortedArray[i * 2 + 1].value = axis.Dot(bounds.maxs);
        sortedArray[i * 2 + 1].isMin = false;
    }

    qsort(sortedArray, num * 2, sizeof(PsuedoBody), CompareSAP);
}

/*
====================================================
BuildPairs
====================================================
*/
void BuildPairs(std::vector<CollisionPair> &collisionPairs, const PsuedoBody *sortedBodies, const int num)
{
    collisionPairs.clear();

    // Now that the bodies are sorted, build the collision pairs
    for (int i = 0; i < num * 2; i++)
    {
        const PsuedoBody &a = sortedBodies[i];
        if (!a.isMin)
        {
            continue;
        }

        CollisionPair pair;
        pair.a = a.id;

        for (int j = i + 1; j < num * 2; j++)
        {
            const PsuedoBody &b = sortedBodies[j];
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
            collisionPairs.push_back(pair);
        }
    }
}

/*
====================================================
SweepAndPrune1D
====================================================
*/
void SweepAndPrune1D(const RigidBody *const *bodies, const int num, std::vector<CollisionPair> &finalPairs,
                     const float dt_sec)
{
    PsuedoBody *sortedBodies = (PsuedoBody *)_malloca(sizeof(PsuedoBody) * num * 2);

    SortBodiesBounds(bodies, num, sortedBodies, dt_sec);
    BuildPairs(finalPairs, sortedBodies, num);
    
    _freea(sortedBodies);
}

void BroadPhase(const RigidBody *const *bodies, const int num, std::vector<CollisionPair> &finalPairs,
                const float dt_sec)
{
    finalPairs.clear();

    SweepAndPrune1D(bodies, num, finalPairs, dt_sec);
}
