#include "pch.h"

#include "ConvexCollider.h"

#include "GJK.h"


// REF: Game Physics In One Week

struct point_t;
float EPA_Expand(const RigidBody *bodyA, const RigidBody *bodyB, const float bias, const point_t simplexPoints[4],
                 Vector3 &ptOnA, Vector3 &ptOnB);

/*
================================
SignedVolume1D
================================
*/
Vector2 SignedVolume1D(const Vector3 &s1, const Vector3 &s2)
{
    Vector3 ab = s2 - s1;                                   // Ray from a to b
    Vector3 ap = Vector3(0.0f) - s1;                        // Ray from a to origin
    Vector3 p0 = s1 + ab * ab.Dot(ap) / ab.LengthSquared(); // projection of the origin onto the line

    // Choose the axis with the greatest difference/length
    int   idx = 0;
    float mu_max = 0;
    for (int i = 0; i < 3; i++)
    {
        float mu = s2[i] - s1[i];
        if (mu * mu > mu_max * mu_max)
        {
            mu_max = mu;
            idx = i;
        }
    }

    // Project the simplex points and projected origin onto the axis with greatest length
    const float a = s1[idx];
    const float b = s2[idx];
    const float p = p0[idx];

    // Get the signed distance from a to p and from p to b
    const float C1 = p - a;
    const float C2 = b - p;

    // if p is between [a,b]
    if ((p > a && p < b) || (p > b && p < a))
    {
        Vector2 lambdas;
        lambdas.x = C2 / mu_max;
        lambdas.y = C1 / mu_max;
        return lambdas;
    }

    // if p is on the far side of a
    if ((a <= b && p <= a) || (a >= b && p >= a))
    {
        return Vector2(1.0f, 0.0f);
    }

    // p must be on the far side of b
    return Vector2(0.0f, 1.0f);
}

/*
================================
CompareSigns
================================
*/
int CompareSigns(float a, float b)
{
    if (a > 0.0f && b > 0.0f)
    {
        return 1;
    }
    if (a < 0.0f && b < 0.0f)
    {
        return 1;
    }
    return 0;
}

/*
================================
SignedVolume2D
================================
*/
Vector3 SignedVolume2D(const Vector3 &s1, const Vector3 &s2, const Vector3 &s3)
{
    Vector3 normal = (s2 - s1).Cross(s3 - s1);
    Vector3 p0 = normal * s1.Dot(normal) / normal.LengthSquared();

    // Find the axis with the greatest projected area
    int   idx = 0;
    float area_max = 0;
    for (int i = 0; i < 3; i++)
    {
        int j = (i + 1) % 3;
        int k = (i + 2) % 3;

        Vector2 a = Vector2(s1[j], s1[k]);
        Vector2 b = Vector2(s2[j], s2[k]);
        Vector2 c = Vector2(s3[j], s3[k]);
        Vector2 ab = b - a;
        Vector2 ac = c - a;

        float area = ab.x * ac.y - ab.y * ac.x;
        if (area * area > area_max * area_max)
        {
            idx = i;
            area_max = area;
        }
    }

    // Project onto the appropriate axis
    int     x = (idx + 1) % 3;
    int     y = (idx + 2) % 3;
    Vector2 s[3];
    s[0] = Vector2(s1[x], s1[y]);
    s[1] = Vector2(s2[x], s2[y]);
    s[2] = Vector2(s3[x], s3[y]);
    Vector2 p = Vector2(p0[x], p0[y]);

    // Get the sub-areas of the triangles formed from the projected origin and the edges
    Vector3 areas;
    for (int i = 0; i < 3; i++)
    {
        int j = (i + 1) % 3;
        int k = (i + 2) % 3;

        Vector2 a = p;
        Vector2 b = s[j];
        Vector2 c = s[k];
        Vector2 ab = b - a;
        Vector2 ac = c - a;

        areas[i] = ab.x * ac.y - ab.y * ac.x;
    }

    // If the projected origin is inside the triangle, then return the barycentric points
    if (CompareSigns(area_max, areas[0]) > 0 && CompareSigns(area_max, areas[1]) > 0 &&
        CompareSigns(area_max, areas[2]) > 0)
    {
        Vector3 lambdas = areas / area_max;
        return lambdas;
    }

    // If we make it here, then we need to project onto the edges and determine the closest point
    float   dist = 1e10;
    Vector3 lambdas = Vector3(1, 0, 0);
    for (int i = 0; i < 3; i++)
    {
        int k = (i + 1) % 3;
        int l = (i + 2) % 3;

        Vector3 edgesPts[3];
        edgesPts[0] = s1;
        edgesPts[1] = s2;
        edgesPts[2] = s3;

        Vector2 lambdaEdge = SignedVolume1D(edgesPts[k], edgesPts[l]);
        Vector3 pt = edgesPts[k] * lambdaEdge.x + edgesPts[l] * lambdaEdge.y;
        if (pt.LengthSquared() < dist)
        {
            dist = pt.LengthSquared();
            lambdas[i] = 0;
            lambdas[k] = lambdaEdge.x;
            lambdas[l] = lambdaEdge.y;
        }
    }

    return lambdas;
}

/*
================================
SignedVolume3D
================================
*/
Vector4 SignedVolume3D(const Vector3 &s1, const Vector3 &s2, const Vector3 &s3, const Vector3 &s4)
{
    Matrix M(s1.x, s2.x, s3.x, s4.x, s1.y, s2.y, s3.y, s4.y, s1.z, s2.z, s3.z, s4.z, 1.0f, 1.0f, 1.0f, 1.0f);

    Vector4 C4;
    C4.x = M.Cofactor(3, 0);
    C4.y = M.Cofactor(3, 1);
    C4.z = M.Cofactor(3, 2);
    C4.w = M.Cofactor(3, 3);

    const float detM = C4.x + C4.y + C4.z + C4.w;

    // If the barycentric coordinates put the origin inside the simplex, then return them
    if (CompareSigns(detM, C4.x) > 0 && CompareSigns(detM, C4.y) > 0 && CompareSigns(detM, C4.z) > 0 &&
        CompareSigns(detM, C4.w) > 0)
    {
        Vector4 lambdas = C4 * (1.0f / detM);
        return lambdas;
    }

    // If we get here, then we need to project the origin onto the faces and determine the closest one
    Vector4 lambdas;
    float   dist = 1e10;
    for (int i = 0; i < 4; i++)
    {
        int j = (i + 1) % 4;
        int k = (i + 2) % 4;

        Vector3 facePts[4];
        facePts[0] = s1;
        facePts[1] = s2;
        facePts[2] = s3;
        facePts[3] = s4;

        Vector3 lambdasFace = SignedVolume2D(facePts[i], facePts[j], facePts[k]);
        Vector3 pt = facePts[i] * lambdasFace[0] + facePts[j] * lambdasFace[1] + facePts[k] * lambdasFace[2];
        if (pt.LengthSquared() < dist)
        {
            dist = pt.LengthSquared();
            lambdas = Vector4::Zero;
            lambdas[i] = lambdasFace[0];
            lambdas[j] = lambdasFace[1];
            lambdas[k] = lambdasFace[2];
        }
    }

    return lambdas;
}

/*
================================================================================================

Gilbert Johnson Keerthi

================================================================================================
*/

struct point_t
{
    Vector3 xyz; // The point on the minkowski sum
    Vector3 ptA; // The point on bodyA
    Vector3 ptB; // The point on bodyB

    point_t() : xyz(0.0f), ptA(0.0f), ptB(0.0f) {}

    const point_t &operator=(const point_t &rhs)
    {
        xyz = rhs.xyz;
        ptA = rhs.ptA;
        ptB = rhs.ptB;
        return *this;
    }

    bool operator==(const point_t &rhs) const { return ((ptA == rhs.ptA) && (ptB == rhs.ptB) && (xyz == rhs.xyz)); }
};

/*
================================
Support
================================
*/
point_t Support(const Collider *pA, const Collider *pB, Vector3 dir, const float bias)
{
    dir.Normalize();

    point_t point;

    // Find the point in A furthest in direction
    point.ptA = pA->Support(dir, pA->Position, pA->Rotation, bias);

    // Find the point in B furthest in the opposite direction
    point.ptB = pB->Support(-dir, pB->Position, pB->Rotation, bias);

    // Return the point, in the minkowski sum, furthest in the direction
    point.xyz = point.ptA - point.ptB;
    return point;
}

/*
================================
SimplexSignedVolumes

Projects the origin onto the simplex to acquire the new search direction,
also checks if the origin is "inside" the simplex.
================================
*/
bool SimplexSignedVolumes(point_t *pts, const int num, Vector3 &newDir, Vector4 &lambdasOut)
{
    const float epsilonf = 0.0001f * 0.0001f;
    lambdasOut = Vector4::Zero;

    bool doesIntersect = false;
    switch (num)
    {
    default:
    case 2: {
        Vector2 lambdas = SignedVolume1D(pts[0].xyz, pts[1].xyz);
        Vector3 v(0.0f);
        v += pts[0].xyz * lambdas.x;
        v += pts[1].xyz * lambdas.y;
        newDir = v * -1.0f;
        doesIntersect = (v.LengthSquared() < epsilonf);
        lambdasOut[0] = lambdas.x;
        lambdasOut[1] = lambdas.y;
    }
    break;
    case 3: {
        Vector3 lambdas = SignedVolume2D(pts[0].xyz, pts[1].xyz, pts[2].xyz);
        Vector3 v(0.0f);
        for (int i = 0; i < 3; i++)
        {
            v += pts[i].xyz * lambdas[i];
        }
        newDir = v * -1.0f;
        doesIntersect = (v.LengthSquared() < epsilonf);
        lambdasOut[0] = lambdas[0];
        lambdasOut[1] = lambdas[1];
        lambdasOut[2] = lambdas[2];
    }
    break;
    case 4: {
        Vector4 lambdas = SignedVolume3D(pts[0].xyz, pts[1].xyz, pts[2].xyz, pts[3].xyz);
        Vector3 v(0.0f);
        for (int i = 0; i < 4; i++)
        {
            v += pts[i].xyz * lambdas[i];
        }
        newDir = v * -1.0f;
        doesIntersect = (v.LengthSquared() < epsilonf);
        lambdasOut[0] = lambdas[0];
        lambdasOut[1] = lambdas[1];
        lambdasOut[2] = lambdas[2];
        lambdasOut[3] = lambdas[3];
    }
    break;
    };

    return doesIntersect;
}

/*
================================
HasPoint

Checks whether the new point already exists in the simplex
================================
*/
bool HasPoint(const point_t simplexPoints[4], const point_t &newPt)
{
    const float precision = 1e-6f;

    for (int i = 0; i < 4; i++)
    {
        Vector3 delta = simplexPoints[i].xyz - newPt.xyz;
        if (delta.LengthSquared() < precision * precision)
        {
            return true;
        }
    }
    return false;
}

/*
================================
SortValids

Sorts the valid support points to the beginning of the array
================================
*/
void SortValids(point_t simplexPoints[4], Vector4 &lambdas)
{
    bool valids[4];
    for (int i = 0; i < 4; i++)
    {
        valids[i] = true;
        if (lambdas[i] == 0.0f)
        {
            valids[i] = false;
        }
    }

    Vector4 validLambdas(0.0f);
    int     validCount = 0;
    point_t validPts[4];
    memset(validPts, 0, sizeof(point_t) * 4);
    for (int i = 0; i < 4; i++)
    {
        if (valids[i])
        {
            validPts[validCount] = simplexPoints[i];
            validLambdas[validCount] = lambdas[i];
            validCount++;
        }
    }

    // Copy the valids back into simplexPoints
    for (int i = 0; i < 4; i++)
    {
        simplexPoints[i] = validPts[i];
        lambdas[i] = validLambdas[i];
    }
}

/*
================================
NumValids
================================
*/
static int NumValids(const Vector4 &lambdas)
{
    int num = 0;
    for (int i = 0; i < 4; i++)
    {
        if (0.0f != lambdas[i])
        {
            num++;
        }
    }
    return num;
}

bool GJK_DoesIntersect(const Collider *pA, const Collider *pB)
{
    const Vector3 origin(0.0f);

    int     numPts = 1;
    point_t simplexPoints[4];
    simplexPoints[0] = Support(pA, pB, Vector3::One, 0.0f);

    float   closestDist = 1e10f;
    bool    doesContainOrigin = false;
    Vector3 newDir = simplexPoints[0].xyz * -1.0f;
    do
    {
        // Get the new point to check on
        point_t newPt = Support(pA, pB, newDir, 0.0f);

        // If the new point is the same as a previous point, then we can't expand any further
        if (HasPoint(simplexPoints, newPt))
        {
            break;
        }

        simplexPoints[numPts] = newPt;
        numPts++;

        // If this new point hasn't moved passed the origin, then the
        // origin cannot be in the set. And therefore there is no collision.
        float dotdot = newDir.Dot(newPt.xyz - origin);
        if (dotdot < 0.0f)
        {
            break;
        }

        Vector4 lambdas;
        doesContainOrigin = SimplexSignedVolumes(simplexPoints, numPts, newDir, lambdas);
        if (doesContainOrigin)
        {
            break;
        }

        // Check that the new projection of the origin onto the simplex is closer than the previous
        float dist = newDir.LengthSquared();
        if (dist >= closestDist)
        {
            break;
        }
        closestDist = dist;

        // Use the lambdas that support the new search direction, and invalidate any points that don't support it
        SortValids(simplexPoints, lambdas);
        numPts = NumValids(lambdas);
        doesContainOrigin = (4 == numPts);
    } while (!doesContainOrigin);

    return doesContainOrigin;
}
