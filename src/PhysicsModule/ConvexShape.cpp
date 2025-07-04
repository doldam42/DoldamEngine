#include "pch.h"
#include "ConvexShape.h"

BOOL ConvexShape::RayTest(const Ray &ray, const Vector3 &pos, const Quaternion &orient, Vector3 *pOutNormal,
                          float *tHit)
{
    for (const tri_t& tri : Triangles)
    {
        Vector3 a = Vector3::Transform(Points[tri.a], orient) + pos;
        Vector3 b = Vector3::Transform(Points[tri.b], orient) + pos;
        Vector3 c = Vector3::Transform(Points[tri.c], orient) + pos;

        float t0;
        if (RayTriangle(ray.position, ray.direction, a, b, c, &t0))
        {
            Vector3 l1 = b - a;
            Vector3 l2 = c - a;
            *pOutNormal = l1.Cross(l2);
            pOutNormal->Normalize();
            *tHit = t0;
            return TRUE;
        }
    }
    return FALSE;
}


/*
================================
FindPointFurthestInDir
================================
*/
int FindPointFurthestInDir(const Vector3 *pts, const int num, const Vector3 &dir)
{
    int   maxIdx = 0;
    float maxDist = dir.Dot(pts[0]);
    for (int i = 1; i < num; i++)
    {
        float dist = dir.Dot(pts[i]);
        if (dist > maxDist)
        {
            maxDist = dist;
            maxIdx = i;
        }
    }
    return maxIdx;
}

/*
================================
DistanceFromLine
================================
*/
float DistanceFromLine(const Vector3 &a, const Vector3 &b, const Vector3 &pt)
{
    Vector3 ab = b - a;
    ab.Normalize();

    Vector3 ray = pt - a;
    Vector3 projection = ab * ray.Dot(ab); // project the ray onto ab
    Vector3 perpindicular = ray - projection;

    return perpindicular.Length();
}

/*
================================
FindPointFurthestFromLine
================================
*/
Vector3 FindPointFurthestFromLine(const Vector3 *pts, const int num, const Vector3 &ptA, const Vector3 &ptB)
{
    int   maxIdx = 0;
    float maxDist = DistanceFromLine(ptA, ptB, pts[0]);
    for (int i = 1; i < num; i++)
    {
        float dist = DistanceFromLine(ptA, ptB, pts[i]);
        if (dist > maxDist)
        {
            maxDist = dist;
            maxIdx = i;
        }
    }
    return pts[maxIdx];
}

/*
================================
DistanceFromTriangle
================================
*/
float DistanceFromTriangle(const Vector3 &a, const Vector3 &b, const Vector3 &c, const Vector3 &pt)
{
    Vector3 ab = b - a;
    Vector3 ac = c - a;
    Vector3 normal = ab.Cross(ac);
    normal.Normalize();

    Vector3 ray = pt - a;
    float   dist = ray.Dot(normal);
    return dist;
}

/*
================================
FindPointFurthestFromTriangle
================================
*/
Vector3 FindPointFurthestFromTriangle(const Vector3 *pts, const int num, const Vector3 &ptA, const Vector3 &ptB,
                                      const Vector3 &ptC)
{
    int   maxIdx = 0;
    float maxDist = DistanceFromTriangle(ptA, ptB, ptC, pts[0]);
    for (int i = 1; i < num; i++)
    {
        float dist = DistanceFromTriangle(ptA, ptB, ptC, pts[i]);
        if (dist * dist > maxDist * maxDist)
        {
            maxDist = dist;
            maxIdx = i;
        }
    }
    return pts[maxIdx];
}

/*
================================
BuildTetrahedron (Quick Hull)
================================
*/
void BuildTetrahedron(const Vector3 *verts, const int num, std::vector<Vector3> &hullPts, std::vector<tri_t> &hullTris)
{
    hullPts.clear();
    hullTris.clear();

    Vector3 points[4];
    int     idx = FindPointFurthestInDir(verts, num, Vector3(1, 0, 0));
    points[0] = verts[idx];
    idx = FindPointFurthestInDir(verts, num, -points[0]);
    points[1] = verts[idx];
    points[2] = FindPointFurthestFromLine(verts, num, points[0], points[1]);
    points[3] = FindPointFurthestFromTriangle(verts, num, points[0], points[1], points[2]);

    // This is important for making sure the ordering is CCW for all faces.
    float dist = DistanceFromTriangle(points[0], points[1], points[2], points[3]);
    if (dist > 0.0f)
    {
        std::swap(points[0], points[1]);
    }

    // Build the tetrahedron
    hullPts.push_back(points[0]);
    hullPts.push_back(points[1]);
    hullPts.push_back(points[2]);
    hullPts.push_back(points[3]);

    tri_t tri;
    tri.a = 0;
    tri.b = 1;
    tri.c = 2;
    hullTris.push_back(tri);

    tri.a = 0;
    tri.b = 2;
    tri.c = 3;
    hullTris.push_back(tri);

    tri.a = 2;
    tri.b = 1;
    tri.c = 3;
    hullTris.push_back(tri);

    tri.a = 1;
    tri.b = 0;
    tri.c = 3;
    hullTris.push_back(tri);
}

void RemoveInternalPoints(const std::vector<Vector3> &hullPoints, const std::vector<tri_t> &hullTris,
                          std::vector<Vector3> &checkPts)
{
    for (int i = 0; i < checkPts.size(); i++)
    {
        const Vector3 &pt = checkPts[i];

        bool isExternal = false;
        for (int t = 0; t < hullTris.size(); t++)
        {
            const tri_t   &tri = hullTris[t];
            const Vector3 &a = hullPoints[tri.a];
            const Vector3 &b = hullPoints[tri.b];
            const Vector3 &c = hullPoints[tri.c];

            // If the point is in front of any triangle then it's external
            float dist = DistanceFromTriangle(a, b, c, pt);
            if (dist > 0.0f)
            {
                isExternal = true;
                break;
            }
        }

        // if it's not external, then it's inside the polyhedron and should be removed
        if (!isExternal)
        {
            checkPts[i] = checkPts.back();
            checkPts.pop_back();
            i--;
        }
    }

    // Also remove any points that are just a little too close to the hull points
    for (int i = 0; i < checkPts.size(); i++)
    {
        const Vector3 &pt = checkPts[i];

        bool isTooClose = false;
        for (int j = 0; j < hullPoints.size(); j++)
        {
            Vector3 hullPt = hullPoints[j];
            Vector3 ray = hullPt - pt;
            if (ray.LengthSquared() < 0.01f * 0.01f)
            { // 1cm is too close
                isTooClose = true;
                break;
            }
        }

        if (isTooClose)
        {
            checkPts[i] = checkPts.back();
            checkPts.pop_back();
            i--;
        }
    }
}

/*
================================
IsEdgeUnique
This will compare the incoming edge with all the edges in the facing tris and then return true if it's unique
================================
*/
bool IsEdgeUnique(const std::vector<tri_t> &tris, const std::vector<int> &facingTris, const int ignoreTri,
                  const edge_t &edge)
{
    for (int i = 0; i < facingTris.size(); i++)
    {
        const int triIdx = facingTris[i];
        if (ignoreTri == triIdx)
        {
            continue;
        }

        const tri_t &tri = tris[triIdx];

        edge_t edges[3];
        edges[0].a = tri.a;
        edges[0].b = tri.b;

        edges[1].a = tri.b;
        edges[1].b = tri.c;

        edges[2].a = tri.c;
        edges[2].b = tri.a;

        for (int e = 0; e < 3; e++)
        {
            if (edge == edges[e])
            {
                return false;
            }
        }
    }
    return true;
}

/*
================================
AddPoint
================================
*/
void AddPoint(std::vector<Vector3> &hullPoints, std::vector<tri_t> &hullTris, const Vector3 &pt)
{
    // This point is outside
    // Now we need to remove old triangles and build new ones

    // Find all the triangles that face this point
    std::vector<int> facingTris;
    for (int i = (int)hullTris.size() - 1; i >= 0; i--)
    {
        const tri_t &tri = hullTris[i];

        const Vector3 &a = hullPoints[tri.a];
        const Vector3 &b = hullPoints[tri.b];
        const Vector3 &c = hullPoints[tri.c];

        const float dist = DistanceFromTriangle(a, b, c, pt);
        if (dist > 0.0f)
        {
            facingTris.push_back(i);
        }
    }

    // Now find all edges that are unique to the tris, these will be the edges that form the new triangles
    std::vector<edge_t> uniqueEdges;
    uniqueEdges.reserve(facingTris.size() * 3);
    for (int i = 0; i < facingTris.size(); i++)
    {
        const int    triIdx = facingTris[i];
        const tri_t &tri = hullTris[triIdx];

        edge_t edges[3];
        edges[0].a = tri.a;
        edges[0].b = tri.b;

        edges[1].a = tri.b;
        edges[1].b = tri.c;

        edges[2].a = tri.c;
        edges[2].b = tri.a;

        for (int e = 0; e < 3; e++)
        {
            if (IsEdgeUnique(hullTris, facingTris, triIdx, edges[e]))
            {
                uniqueEdges.push_back(edges[e]);
            }
        }
    }

    // now remove the old facing tris
    for (int i = 0; i < facingTris.size(); i++)
    {
        hullTris[facingTris[i]] = hullTris.back();
        hullTris.pop_back();
    }

    // Now add the new point
    hullPoints.push_back(pt);
    const int newPtIdx = (int)hullPoints.size() - 1;

    // Now add triangles for each unique edge
    for (int i = 0; i < uniqueEdges.size(); i++)
    {
        const edge_t &edge = uniqueEdges[i];

        tri_t tri;
        tri.a = edge.a;
        tri.b = edge.b;
        tri.c = newPtIdx;
        hullTris.push_back(tri);
    }
}

/*
================================
RemoveUnreferencedVerts
================================
*/
void RemoveUnreferencedVerts(std::vector<Vector3> &hullPoints, std::vector<tri_t> &hullTris)
{
    for (int i = 0; i < hullPoints.size(); i++)
    {

        bool isUsed = false;
        for (int j = 0; j < hullTris.size(); j++)
        {
            const tri_t &tri = hullTris[j];

            if (tri.a == i || tri.b == i || tri.c == i)
            {
                isUsed = true;
                break;
            }
        }

        if (isUsed)
        {
            continue;
        }

        for (int j = 0; j < hullTris.size(); j++)
        {
            tri_t &tri = hullTris[j];
            if (tri.a > i)
            {
                tri.a--;
            }
            if (tri.b > i)
            {
                tri.b--;
            }
            if (tri.c > i)
            {
                tri.c--;
            }
        }

        hullPoints[i] = hullPoints.back();
        hullPoints.pop_back();
        i--;
    }
}

/*
================================
ExpandConvexHull
================================
*/
void ExpandConvexHull(std::vector<Vector3> &hullPoints, std::vector<tri_t> &hullTris, const std::vector<Vector3> &verts)
{
    std::vector<Vector3> externalVerts = verts;
    RemoveInternalPoints(hullPoints, hullTris, externalVerts);

    while (externalVerts.size() > 0)
    {
        int ptIdx = FindPointFurthestInDir(externalVerts.data(), (int)externalVerts.size(), externalVerts[0]);

        Vector3 pt = externalVerts[ptIdx];

        // remove this element
        externalVerts[ptIdx] = externalVerts.back();
        externalVerts.pop_back();

        AddPoint(hullPoints, hullTris, pt);

        RemoveInternalPoints(hullPoints, hullTris, externalVerts);
    }

    RemoveUnreferencedVerts(hullPoints, hullTris);
}

/*
================================
IsExternal
================================
*/
bool IsExternal(const std::vector<Vector3> &pts, const std::vector<tri_t> &tris, const Vector3 &pt)
{
    bool isExternal = false;
    for (int t = 0; t < tris.size(); t++)
    {
        const tri_t   &tri = tris[t];
        const Vector3 &a = pts[tri.a];
        const Vector3 &b = pts[tri.b];
        const Vector3 &c = pts[tri.c];

        // If the point is in front of any triangle then it's external
        float dist = DistanceFromTriangle(a, b, c, pt);
        if (dist > 0.0f)
        {
            isExternal = true;
            break;
        }
    }

    return isExternal;
}

/*
================================
CalculateCenterOfMass
================================
*/
Vector3 CalculateCenterOfMass(const std::vector<Vector3> &pts, const std::vector<tri_t> &tris)
{
    const int numSamples = 100;

    Bounds bounds;
    bounds.Expand(pts.data(), pts.size());

    Vector3     cm(0.0f);
    const float dx = bounds.WidthX() / (float)numSamples;
    const float dy = bounds.WidthY() / (float)numSamples;
    const float dz = bounds.WidthZ() / (float)numSamples;

    int sampleCount = 0;
    for (float x = bounds.mins.x; x < bounds.maxs.x; x += dx)
    {
        for (float y = bounds.mins.y; y < bounds.maxs.y; y += dy)
        {
            for (float z = bounds.mins.z; z < bounds.maxs.z; z += dz)
            {
                Vector3 pt(x, y, z);

                if (IsExternal(pts, tris, pt))
                {
                    continue;
                }

                cm += pt;
                sampleCount++;
            }
        }
    }

    cm /= (float)sampleCount;
    return cm;
}

/*
================================
CalculateInertiaTensor
================================
*/
Matrix CalculateInertiaTensor(const std::vector<Vector3> &pts, const std::vector<tri_t> &tris, const Vector3 &cm)
{
    const int numSamples = 100;

    Bounds bounds;
    bounds.Expand(pts.data(), (int)pts.size());

    Matrix tensor;

    const float dx = bounds.WidthX() / (float)numSamples;
    const float dy = bounds.WidthY() / (float)numSamples;
    const float dz = bounds.WidthZ() / (float)numSamples;

    int sampleCount = 0;
    for (float x = bounds.mins.x; x < bounds.maxs.x; x += dx)
    {
        for (float y = bounds.mins.y; y < bounds.maxs.y; y += dy)
        {
            for (float z = bounds.mins.z; z < bounds.maxs.z; z += dz)
            {
                Vector3 pt(x, y, z);

                if (IsExternal(pts, tris, pt))
                {
                    continue;
                }

                // Get the point relative to the center of mass
                pt -= cm;

                tensor._11 += pt.y * pt.y + pt.z * pt.z;
                tensor._22 += pt.z * pt.z + pt.x * pt.x;
                tensor._33 += pt.x * pt.x + pt.y * pt.y;

                tensor._12 += -1.0f * pt.x * pt.y;
                tensor._13 += -1.0f * pt.x * pt.z;
                tensor._23 += -1.0f * pt.y * pt.z;

                tensor._21 += -1.0f * pt.x * pt.y;
                tensor._31 += -1.0f * pt.x * pt.z;
                tensor._32 += -1.0f * pt.y * pt.z;

                sampleCount++;
            }
        }
    }

    tensor *= 1.0f / (float)sampleCount;
    return tensor;
}

/*
================================
BuildConvexHull
================================
*/
void BuildConvexHull(const std::vector<Vector3> &verts, std::vector<Vector3> &hullPts, std::vector<tri_t> &hullTris)
{
    if (verts.size() < 4)
    {
        return;
    }

    // Build a tetrahedron
    BuildTetrahedron(verts.data(), (int)verts.size(), hullPts, hullTris);

    ExpandConvexHull(hullPts, hullTris, verts);
}

ConvexShape::ConvexShape(const Vector3 *points, const int numPoints)
{
    std::vector<Vector3> hullPoints;
    std::vector<tri_t>   hullTriangles;
    hullPoints.reserve(numPoints);
    hullTriangles.reserve(numPoints);

    // Expand into a convex hull
    BuildConvexHull(std::vector<Vector3>(points, points + numPoints), hullPoints, hullTriangles);

    // Expand the bounds
    AABB.Clear();
    AABB.Expand(hullPoints.data(), hullPoints.size());

    // m_centerOfMass = CalculateCenterOfMass(hullPoints, hullTriangles);

    // m_inertiaTensor = CalculateInertiaTensor(hullPoints, hullTriangles, m_centerOfMass);

    Points = std::move(hullPoints);
    Triangles = std::move(hullTriangles);
}

Vector3 ConvexShape::Support(const Vector3& dir, const Vector3& pos, const Quaternion& orient, const float bias) const
{
    Vector3 maxPt = Vector3::Transform(Points[0], orient) + pos;
    float   maxDist = maxPt.Dot(dir);
    for (size_t i = 1; i < Points.size(); ++i)
    {
        Vector3 pt = Vector3::Transform(Points[i], orient) + pos;
        float   dist = pt.Dot(dir);
        if (dist > maxDist)
        {
            maxDist = dist;
            maxPt = pt;
        }
    }

    Vector3 norm = dir;
    norm.Normalize();
    norm *= bias;

    return maxPt + norm;
}
