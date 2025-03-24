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
    Vector3 ab = s2 - s1;                                  // Ray from a to b
    Vector3 ap = Vector3(0.0f) - s1;                          // Ray from a to origin
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
    int  x = (idx + 1) % 3;
    int  y = (idx + 2) % 3;
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
    float dist = 1e10;
    Vector3  lambdas = Vector3(1, 0, 0);
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
    Vector4  lambdas;
    float dist = 1e10;
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
point_t Support(const RigidBody *bodyA, const RigidBody *bodyB, Vector3 dir, const float bias)
{
    dir.Normalize();

    point_t point;

    // Find the point in A furthest in direction
    point.ptA = bodyA->m_pCollider->Support(dir, bodyA->GetPosition(), bodyA->GetOrient(), bias);

    // Find the point in B furthest in the opposite direction
    point.ptB = bodyB->m_pCollider->Support(-dir, bodyB->GetPosition(), bodyB->GetOrient(), bias);

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

    Vector4    validLambdas(0.0f);
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

/*
================================
GJK_DoesIntersect
================================
*/
bool GJK_DoesIntersect(const RigidBody *bodyA, const RigidBody *bodyB)
{
    const Vector3 origin(0.0f);

    int     numPts = 1;
    point_t simplexPoints[4];
    simplexPoints[0] = Support(bodyA, bodyB, Vector3::One, 0.0f);

    float closestDist = 1e10f;
    bool  doesContainOrigin = false;
    Vector3  newDir = simplexPoints[0].xyz * -1.0f;
    do
    {
        // Get the new point to check on
        point_t newPt = Support(bodyA, bodyB, newDir, 0.0f);

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

/*
================================
GJK_ClosestPoints
================================
*/
void GJK_ClosestPoints(const RigidBody *bodyA, const RigidBody *bodyB, Vector3 &ptOnA, Vector3 &ptOnB)
{
    const Vector3 origin(0.0f);

    float       closestDist = 1e10f;
    const float bias = 0.0f;

    int     numPts = 1;
    point_t simplexPoints[4];
    simplexPoints[0] = Support(bodyA, bodyB, Vector3(1, 1, 1), bias);

    Vector4 lambdas = Vector4(1, 0, 0, 0);
    Vector3 newDir = simplexPoints[0].xyz * -1.0f;
    do
    {
        // Get the new point to check on
        point_t newPt = Support(bodyA, bodyB, newDir, bias);

        // If the new point is the same as a previous point, then we can't expand any further
        if (HasPoint(simplexPoints, newPt))
        {
            break;
        }

        // Add point and get new search direction
        simplexPoints[numPts] = newPt;
        numPts++;

        SimplexSignedVolumes(simplexPoints, numPts, newDir, lambdas);
        SortValids(simplexPoints, lambdas);
        numPts = NumValids(lambdas);

        // Check that the new projection of the origin onto the simplex is closer than the previous
        float dist = newDir.LengthSquared();
        if (dist >= closestDist)
        {
            break;
        }
        closestDist = dist;
    } while (numPts < 4);

    ptOnA = Vector3::Zero;
    ptOnB = Vector3::Zero;
    for (int i = 0; i < 4; i++)
    {
        ptOnA += simplexPoints[i].ptA * lambdas[i];
        ptOnB += simplexPoints[i].ptB * lambdas[i];
    }
}


/*
================================
GJK_DoesIntersect
================================
*/
bool GJK_DoesIntersect(const RigidBody *bodyA, const RigidBody *bodyB, const float bias, Vector3 &ptOnA, Vector3 &ptOnB)
{
    const Vector3 origin(0.0f);

    int     numPts = 1;
    point_t simplexPoints[4];
    simplexPoints[0] = Support(bodyA, bodyB, Vector3(1, 1, 1), 0.0f);

    float closestDist = 1e10f;
    bool  doesContainOrigin = false;
    Vector3  newDir = simplexPoints[0].xyz * -1.0f;
    do
    {
        // Get the new point to check on
        point_t newPt = Support(bodyA, bodyB, newDir, 0.0f);

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

    if (!doesContainOrigin)
    {
        return false;
    }

    //
    //	Check that we have a 3-simplex (EPA expects a tetrahedron)
    //
    if (1 == numPts)
    {
        Vector3    searchDir = simplexPoints[0].xyz * -1.0f;
        point_t newPt = Support(bodyA, bodyB, searchDir, 0.0f);
        simplexPoints[numPts] = newPt;
        numPts++;
    }
    if (2 == numPts)
    {
        Vector3 ab = simplexPoints[1].xyz - simplexPoints[0].xyz;
        Vector3 u, v;
        ab.GetOrthoNormal(&u, &v);

        Vector3    newDir = u;
        point_t newPt = Support(bodyA, bodyB, newDir, 0.0f);
        simplexPoints[numPts] = newPt;
        numPts++;
    }
    if (3 == numPts)
    {
        Vector3 ab = simplexPoints[1].xyz - simplexPoints[0].xyz;
        Vector3 ac = simplexPoints[2].xyz - simplexPoints[0].xyz;
        Vector3 norm = ab.Cross(ac);

        Vector3    newDir = norm;
        point_t newPt = Support(bodyA, bodyB, newDir, 0.0f);
        simplexPoints[numPts] = newPt;
        numPts++;
    }

    //
    // Expand the simplex by the bias amount
    //

    // Get the center point of the simplex
    Vector3 avg = Vector3(0, 0, 0);
    for (int i = 0; i < 4; i++)
    {
        avg += simplexPoints[i].xyz;
    }
    avg *= 0.25f;

    // Now expand the simplex by the bias amount
    for (int i = 0; i < numPts; i++)
    {
        point_t &pt = simplexPoints[i];

        Vector3 dir = pt.xyz - avg; // ray from "center" to witness point
        dir.Normalize();
        pt.ptA += dir * bias;
        pt.ptB -= dir * bias;
        pt.xyz = pt.ptA - pt.ptB;
    }

    //
    // Perform EPA expansion of the simplex to find the closest face on the CSO
    //
    EPA_Expand(bodyA, bodyB, bias, simplexPoints, ptOnA, ptOnB);
    return true;
}

/*
================================================================================================

Expanding Polytope Algorithm

================================================================================================
*/

/*
================================
BarycentricCoordinates

This borrows our signed volume code to perform the barycentric coordinates.
================================
*/
Vector3 BarycentricCoordinates(Vector3 s1, Vector3 s2, Vector3 s3, const Vector3 &pt)
{
    s1 = s1 - pt;
    s2 = s2 - pt;
    s3 = s3 - pt;

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
    int  x = (idx + 1) % 3;
    int  y = (idx + 2) % 3;
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

    Vector3 lambdas = areas / area_max;
    if (!lambdas.IsValid())
    {
        lambdas = Vector3(1, 0, 0);
    }
    return lambdas;
}

/*
================================
NormalDirection
================================
*/
Vector3 NormalDirection(const tri_t &tri, const std::vector<point_t> &points)
{
    const Vector3 &a = points[tri.a].xyz;
    const Vector3 &b = points[tri.b].xyz;
    const Vector3 &c = points[tri.c].xyz;

    Vector3 ab = b - a;
    Vector3 ac = c - a;
    Vector3 normal = ab.Cross(ac);
    normal.Normalize();
    return normal;
}

/*
================================
SignedDistanceToTriangle
================================
*/
float SignedDistanceToTriangle(const tri_t &tri, const Vector3 &pt, const std::vector<point_t> &points)
{
    const Vector3  normal = NormalDirection(tri, points);
    const Vector3 &a = points[tri.a].xyz;
    const Vector3  a2pt = pt - a;
    const float dist = normal.Dot(a2pt);
    return dist;
}

/*
================================
ClosestTriangle
================================
*/
int ClosestTriangle(const std::vector<tri_t> &triangles, const std::vector<point_t> &points)
{
    float minDistSqr = 1e10;

    int idx = -1;
    for (int i = 0; i < triangles.size(); i++)
    {
        const tri_t &tri = triangles[i];

        float dist = SignedDistanceToTriangle(tri, Vector3(0.0f), points);
        float distSqr = dist * dist;
        if (distSqr < minDistSqr)
        {
            idx = i;
            minDistSqr = distSqr;
        }
    }

    return idx;
}

/*
================================
HasPoint
================================
*/
bool HasPoint(const Vector3 &w, const std::vector<tri_t> triangles, const std::vector<point_t> &points)
{
    const float epsilons = 0.001f * 0.001f;
    Vector3        delta;

    for (int i = 0; i < triangles.size(); i++)
    {
        const tri_t &tri = triangles[i];

        delta = w - points[tri.a].xyz;
        if (delta.LengthSquared() < epsilons)
        {
            return true;
        }
        delta = w - points[tri.b].xyz;
        if (delta.LengthSquared() < epsilons)
        {
            return true;
        }
        delta = w - points[tri.c].xyz;
        if (delta.LengthSquared() < epsilons)
        {
            return true;
        }
    }
    return false;
}

/*
================================
RemoveTrianglesFacingPoint
================================
*/
int RemoveTrianglesFacingPoint(const Vector3 &pt, std::vector<tri_t> &triangles, const std::vector<point_t> &points)
{
    int numRemoved = 0;
    for (int i = 0; i < triangles.size(); i++)
    {
        const tri_t &tri = triangles[i];

        float dist = SignedDistanceToTriangle(tri, pt, points);
        if (dist > 0.0f)
        {
            // This triangle faces the point.  Remove it.
            triangles[i] = triangles.back();
            triangles.pop_back();
            i--;
            numRemoved++;
        }
    }
    return numRemoved;
}

/*
================================
FindDanglingEdges
================================
*/
void FindDanglingEdges(std::vector<edge_t> &danglingEdges, const std::vector<tri_t> &triangles)
{
    danglingEdges.clear();

    for (int i = 0; i < triangles.size(); i++)
    {
        const tri_t &tri = triangles[i];

        edge_t edges[3];
        edges[0].a = tri.a;
        edges[0].b = tri.b;

        edges[1].a = tri.b;
        edges[1].b = tri.c;

        edges[2].a = tri.c;
        edges[2].b = tri.a;

        int counts[3];
        counts[0] = 0;
        counts[1] = 0;
        counts[2] = 0;

        for (int j = 0; j < triangles.size(); j++)
        {
            if (j == i)
            {
                continue;
            }

            const tri_t &tri2 = triangles[j];

            edge_t edges2[3];
            edges2[0].a = tri2.a;
            edges2[0].b = tri2.b;

            edges2[1].a = tri2.b;
            edges2[1].b = tri2.c;

            edges2[2].a = tri2.c;
            edges2[2].b = tri2.a;

            for (int k = 0; k < 3; k++)
            {
                if (edges[k] == edges2[0])
                {
                    counts[k]++;
                }
                if (edges[k] == edges2[1])
                {
                    counts[k]++;
                }
                if (edges[k] == edges2[2])
                {
                    counts[k]++;
                }
            }
        }

        // An edge that isn't shared, is dangling
        for (int k = 0; k < 3; k++)
        {
            if (0 == counts[k])
            {
                danglingEdges.push_back(edges[k]);
            }
        }
    }
}

/*
================================
EPA_Expand
================================
*/
float EPA_Expand(const RigidBody *bodyA, const RigidBody *bodyB, const float bias, const point_t simplexPoints[4],
                 Vector3 &ptOnA,
                 Vector3 &ptOnB)
{
    std::vector<point_t> points(4);
    std::vector<tri_t>   triangles(4);
    std::vector<edge_t>  danglingEdges(4 * 3);

    Vector3 center(0.0f);
    for (int i = 0; i < 4; i++)
    {
        points[i] = simplexPoints[i];
        center += simplexPoints[i].xyz;
    }
    center *= 0.25f;

    // Build the triangles
    for (int i = 0; i < 4; i++)
    {
        int   j = (i + 1) % 4;
        int   k = (i + 2) % 4;
        tri_t tri;
        tri.a = i;
        tri.b = j;
        tri.c = k;

        int   unusedPt = (i + 3) % 4;
        float dist = SignedDistanceToTriangle(tri, points[unusedPt].xyz, points);

        // The unused point is always on the negative/inside of the triangle.. make sure the normal points away
        if (dist > 0.0f)
        {
            std::swap(tri.a, tri.b);
        }

        triangles[i] = tri;
    }

    //
    //	Expand the simplex to find the closest face of the CSO to the origin
    //
    while (1)
    {
        const int idx = ClosestTriangle(triangles, points);
        Vector3      normal = NormalDirection(triangles[idx], points);

        const point_t newPt = Support(bodyA, bodyB, normal, bias);

        // if w already exists, then just stop
        // because it means we can't expand any further
        if (HasPoint(newPt.xyz, triangles, points))
        {
            break;
        }

        float dist = SignedDistanceToTriangle(triangles[idx], newPt.xyz, points);
        if (dist <= 0.0f)
        {
            break; // can't expand
        }

        const int newIdx = (int)points.size();
        points.push_back(newPt);

        // Remove Triangles that face this point
        int numRemoved = RemoveTrianglesFacingPoint(newPt.xyz, triangles, points);
        if (0 == numRemoved)
        {
            break;
        }

        // Find Dangling Edges
        danglingEdges.clear();
        FindDanglingEdges(danglingEdges, triangles);
        if (0 == danglingEdges.size())
        {
            break;
        }

        // In theory the edges should be a proper CCW order
        // So we only need to add the new point as 'a' in order
        // to create new triangles that face away from origin
        for (int i = 0; i < danglingEdges.size(); i++)
        {
            const edge_t &edge = danglingEdges[i];

            tri_t triangle;
            triangle.a = newIdx;
            triangle.b = edge.b;
            triangle.c = edge.a;

            // Make sure it's oriented properly
            float dist = SignedDistanceToTriangle(triangle, center, points);
            if (dist > 0.0f)
            {
                std::swap(triangle.b, triangle.c);
            }

            triangles.push_back(triangle);
        }
    }

    // Get the projection of the origin on the closest triangle
    const int    idx = ClosestTriangle(triangles, points);
    const tri_t &tri = triangles[idx];
    Vector3         ptA_w = points[tri.a].xyz;
    Vector3         ptB_w = points[tri.b].xyz;
    Vector3         ptC_w = points[tri.c].xyz;
    Vector3         lambdas = BarycentricCoordinates(ptA_w, ptB_w, ptC_w, Vector3(0.0f));

    // Get the point on shape A
    Vector3 ptA_a = points[tri.a].ptA;
    Vector3 ptB_a = points[tri.b].ptA;
    Vector3 ptC_a = points[tri.c].ptA;
    ptOnA = ptA_a * lambdas[0] + ptB_a * lambdas[1] + ptC_a * lambdas[2];

    // Get the point on shape B
    Vector3 ptA_b = points[tri.a].ptB;
    Vector3 ptB_b = points[tri.b].ptB;
    Vector3 ptC_b = points[tri.c].ptB;
    ptOnB = ptA_b * lambdas[0] + ptB_b * lambdas[1] + ptC_b * lambdas[2];

    // Return the penetration distance
    Vector3 delta = ptOnB - ptOnA;
    return delta.Length();
}