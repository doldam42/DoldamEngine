#include "pch.h"
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
        const Collider *body = bodies[i];
        Bounds           bounds = body->GetBounds();

        // Expand the bounds by the linear velocity
        bounds.Expand(bounds.mins + body->m_linearVelocity * dt_sec);
        bounds.Expand(bounds.maxs + body->m_linearVelocity * dt_sec);

        const float epsilon = 0.01f;
        bounds.Expand(bounds.mins + Vector3(-1, -1, -1) * epsilon);
        bounds.Expand(bounds.maxs + Vector3(1, 1, 1) * epsilon);

        sortedArray[i * 2 + 0].id = i;
        sortedArray[i * 2 + 0].value = m_axisSAP.Dot(bounds.mins);
        sortedArray[i * 2 + 0].isMin = true;

        sortedArray[i * 2 + 1].id = i;
        sortedArray[i * 2 + 1].value = m_axisSAP.Dot(bounds.maxs);
        sortedArray[i * 2 + 1].isMin = false;
    }

    qsort(sortedArray, num * 2, sizeof(PsuedoBody), CompareSAP);
}