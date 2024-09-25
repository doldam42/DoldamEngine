#include "pch.h"

#include "MeshObject.h"

void MeshObject::InitCollisionBox()
{
    if (m_header.VertexCount == 0)
    {
        __debugbreak();
        return;
    }

    Vector3 minCorner;
    Vector3 maxCorner;

    Vector3 center;
    Vector3 extents;

    BasicVertex *pBasicVertex = nullptr;

    pBasicVertex = GetBasicVertices();
    minCorner = maxCorner = pBasicVertex[0].position;

    for (int i = 0; i < GetVertexCount(); i++)
    {
        minCorner = Vector3::Min(minCorner, pBasicVertex[i].position);
        maxCorner = Vector3::Max(maxCorner, pBasicVertex[i].position);
    }

    center = (minCorner + maxCorner) * 0.5f;
    extents = maxCorner - center;

    m_boundingBox = Box(minCorner, maxCorner);
    m_pCollisionBoxMesh = CreateWireBoxMesh(center, extents + Vector3(1e-2));
}

void MeshObject::InitCollisionSphere()
{
    if (m_header.VertexCount == 0)
    {
        __debugbreak();
        return;
    }

    BasicVertex *pBasicVertex = nullptr;

    float maxRadius = 0.0f;
    pBasicVertex = GetBasicVertices();
    for (int i = 0; i < GetVertexCount(); i++)
    {
        float radius = (m_boundingBox.GetCenter() - pBasicVertex[i].position).Length();
        maxRadius = (maxRadius < radius) ? radius : maxRadius;
    }
    maxRadius += 1e-2f; // 살짝 크게 설정
    m_boundingSphere = Sphere(m_boundingBox.GetCenter(), maxRadius);

    m_pCollisionSphereMesh = CreateSphereMesh(maxRadius, 8, 8);
}

IRenderMesh *MeshObject::CreateWireBoxMesh(const Vector3 &center, const Vector3 &extends)
{
    std::vector<Vector3> positions;
    std::vector<Vector3> normals;

    positions.reserve(24);
    normals.reserve(24);

    // 윗면
    positions.push_back(center + Vector3(-1.0f, 1.0f, -1.0f) * extends);
    positions.push_back(center + Vector3(-1.0f, 1.0f, 1.0f) * extends);
    positions.push_back(center + Vector3(1.0f, 1.0f, 1.0f) * extends);
    positions.push_back(center + Vector3(1.0f, 1.0f, -1.0f) * extends);
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f));

    // 아랫면
    positions.push_back(center + Vector3(-1.0f, -1.0f, -1.0f) * extends);
    positions.push_back(center + Vector3(1.0f, -1.0f, -1.0f) * extends);
    positions.push_back(center + Vector3(1.0f, -1.0f, 1.0f) * extends);
    positions.push_back(center + Vector3(-1.0f, -1.0f, 1.0f) * extends);
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f));

    // 앞면
    positions.push_back(center + Vector3(-1.0f, -1.0f, -1.0f) * extends);
    positions.push_back(center + Vector3(-1.0f, 1.0f, -1.0f) * extends);
    positions.push_back(center + Vector3(1.0f, 1.0f, -1.0f) * extends);
    positions.push_back(center + Vector3(1.0f, -1.0f, -1.0f) * extends);
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));

    // 뒷면
    positions.push_back(center + Vector3(-1.0f, -1.0f, 1.0f) * extends);
    positions.push_back(center + Vector3(1.0f, -1.0f, 1.0f) * extends);
    positions.push_back(center + Vector3(1.0f, 1.0f, 1.0f) * extends);
    positions.push_back(center + Vector3(-1.0f, 1.0f, 1.0f) * extends);
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f));

    // 왼쪽
    positions.push_back(center + Vector3(-1.0f, -1.0f, 1.0f) * extends);
    positions.push_back(center + Vector3(-1.0f, 1.0f, 1.0f) * extends);
    positions.push_back(center + Vector3(-1.0f, 1.0f, -1.0f) * extends);
    positions.push_back(center + Vector3(-1.0f, -1.0f, -1.0f) * extends);
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));

    // 오른쪽
    positions.push_back(center + Vector3(1.0f, -1.0f, 1.0f) * extends);
    positions.push_back(center + Vector3(1.0f, -1.0f, -1.0f) * extends);
    positions.push_back(center + Vector3(1.0f, 1.0f, -1.0f) * extends);
    positions.push_back(center + Vector3(1.0f, 1.0f, 1.0f) * extends);
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f));

    BasicVertex pVertices[24];
    for (size_t i = 0; i < 24; i++)
    {
        BasicVertex *v = pVertices + i;
        v->position = positions[i];
        v->normal = normals[i];
        v->texcoord = Vector2(0.0f);
    }

    uint32_t indices[] = {
        0,  1,  2,  0,  2,  3,  // 윗면
        4,  5,  6,  4,  6,  7,  // 아랫면
        8,  9,  10, 8,  10, 11, // 앞면
        12, 13, 14, 12, 14, 15, // 뒷면
        16, 17, 18, 16, 18, 19, // 왼쪽
        20, 21, 22, 20, 22, 23  // 오른쪽
    };

    Material     defaultMaterial = Material();
    IRenderMesh *pMesh = m_pRenderer->CreateMeshObject();
    m_pRenderer->BeginCreateMesh(pMesh, pVertices, 24, 1, nullptr);
    m_pRenderer->InsertFaceGroup(pMesh, indices, 12, &defaultMaterial);
    m_pRenderer->EndCreateMesh(pMesh);

    return pMesh;
}

IRenderMesh *MeshObject::CreateSphereMesh(float radius, int numSlices, int numStacks)
{
    const float dTheta = -XM_2PI / float(numSlices);
    const float dPhi = -XM_PI / float(numStacks);

    std::vector<BasicVertex> vertices;
    vertices.reserve(numStacks * numSlices);

    for (int j = 0; j <= numStacks; j++)
    {
        // 스택에 쌓일 수록 시작점을 x-y 평면에서 회전 시켜서 위로 올리는 구조
        Vector3 stackStartPoint = Vector3::Transform(Vector3(0.0f, -radius, 0.0f), Matrix::CreateRotationZ(dPhi * j));

        for (int i = 0; i <= numSlices; i++)
        {
            BasicVertex v;

            // 시작점을 x-z 평면에서 회전시키면서 원을 만드는 구조
            v.position = Vector3::Transform(stackStartPoint, Matrix::CreateRotationY(dTheta * float(i)));

            v.normal = v.position; // 원점이 구의 중심
            v.normal.Normalize();
            v.texcoord = Vector2(float(i) / numSlices, 1.0f - float(j) / numStacks);

            vertices.push_back(v);
        }
    }

    std::vector<uint32_t> indices;
    indices.reserve(numStacks * numSlices * 6);
    for (int j = 0; j < numStacks; j++)
    {

        const int offset = (numSlices + 1) * j;

        for (int i = 0; i < numSlices; i++)
        {

            indices.push_back(offset + i);
            indices.push_back(offset + i + numSlices + 1);
            indices.push_back(offset + i + 1 + numSlices + 1);

            indices.push_back(offset + i);
            indices.push_back(offset + i + 1 + numSlices + 1);
            indices.push_back(offset + i + 1);
        }
    }

    Material     defaultMaterial = Material();
    IRenderMesh *pMesh = m_pRenderer->CreateMeshObject();
    m_pRenderer->BeginCreateMesh(pMesh, vertices.data(), vertices.size(), 1, nullptr);
    m_pRenderer->InsertFaceGroup(pMesh, indices.data(), indices.size() / 3, &defaultMaterial);
    m_pRenderer->EndCreateMesh(pMesh);
    return pMesh;
}

void MeshObject::Cleanup()
{
    if (m_pCollisionBoxMesh)
    {
        m_pCollisionBoxMesh->Release();
        m_pCollisionBoxMesh = nullptr;
    }
    if (m_pCollisionSphereMesh)
    {
        m_pCollisionSphereMesh->Release();
        m_pCollisionSphereMesh = nullptr;
    }

    if (m_pMeshHandle)
    {
        m_pMeshHandle->Release();
        m_pMeshHandle = nullptr;
    }

    if (m_pVertices)
    {
        delete[] m_pVertices;
        m_pVertices = nullptr;
    }

    if (m_ppFaceGroup)
    {
        for (int i = 0; i < m_header.FaceGroupCount; i++)
        {
            FACE_GROUP::Dealloc(m_ppFaceGroup[i]);
            m_ppFaceGroup[i] = nullptr;
        }
        delete[] m_ppFaceGroup;
        m_ppFaceGroup = nullptr;
    }
}

void MeshObject::InitMeshHandle(IRenderer *pRnd, const Material *pMaterials, const wchar_t *basePath)
{
    IRenderMesh *pHandle = pRnd->CreateMeshObject();
    pRnd->BeginCreateMesh(pHandle, GetBasicVertices(), m_header.VertexCount, m_header.FaceGroupCount, basePath);
    for (uint32_t i = 0; i < m_header.FaceGroupCount; i++)
    {
        FACE_GROUP *pFace = GetFaceGroup(i);
        Material    material = pMaterials[pFace->Header.MaterialIndex];

        pRnd->InsertFaceGroup(pHandle, pFace->pIndices, pFace->Header.IndexCount / 3, &material);
    }
    pRnd->EndCreateMesh(pHandle);
    m_pMeshHandle = pHandle;
    m_pRenderer = pRnd;

    InitCollisionBox();
    InitCollisionSphere();
}

void MeshObject::BeginCreateMesh(const void *pVertices, VERTEX_TYPE vertexType, uint32_t numVertices,
                                 uint32_t numFaceGroup)
{
    m_header.FaceGroupCount = 0;
    m_header.VertexCount = numVertices;
    m_header.Type = vertexType;

    size_t vertexSize = (vertexType == VERTEX_TYPE_BASIC) ? sizeof(BasicVertex) : sizeof(SkinnedVertex);
    m_pVertices = new uint8_t[vertexSize * numVertices];
    memcpy(m_pVertices, pVertices, vertexSize * numVertices);

    m_ppFaceGroup = new FACE_GROUP *[numFaceGroup];
}

void MeshObject::InsertFaceGroup(const uint32_t *pIndices, uint32_t numIndices, int materialIdx)
{
    FACE_GROUP_HEADER header;
    header.IndexCount = numIndices;
    header.MaterialIndex = materialIdx;
    FACE_GROUP *pFace = FACE_GROUP::Alloc(header);

    memcpy(pFace->pIndices, pIndices, sizeof(uint32_t) * numIndices);

    m_ppFaceGroup[m_header.FaceGroupCount] = pFace;
    m_header.FaceGroupCount++;
}

// MESH OBJECT의 FILE 구조
// | VERTEX_STREAM | FACE_GROUP_STREAM1 | FACE_GROUP_STREAM2 | ... |
void MeshObject::WriteFile(FILE *fp)
{
    BasicObject::WriteFile(fp);

    fwrite(&m_header, sizeof(MESH_HEADER), 1, fp);

    size_t vertexSize = (m_header.Type == VERTEX_TYPE_BASIC) ? sizeof(BasicVertex) : sizeof(SkinnedVertex);
    fwrite(m_pVertices, vertexSize, m_header.VertexCount, fp);
    for (int i = 0; i < m_header.FaceGroupCount; i++)
    {
        FACE_GROUP *pFace = m_ppFaceGroup[i];
        fwrite(pFace, pFace->GetSize(), 1, fp);
    }
}

void MeshObject::ReadFile(FILE *fp)
{
    MESH_HEADER  meshHeader;
    uint8_t     *pVertices = nullptr;
    FACE_GROUP **ppFaces = nullptr;

    fread(&meshHeader, sizeof(MESH_HEADER), 1, fp);

    size_t vertexSize = (meshHeader.Type == VERTEX_TYPE_BASIC) ? sizeof(BasicVertex) : sizeof(SkinnedVertex);
    pVertices = new uint8_t[vertexSize * meshHeader.VertexCount];
    fread(pVertices, vertexSize, meshHeader.VertexCount, fp);

    ppFaces = new FACE_GROUP *[meshHeader.FaceGroupCount];
    for (int i = 0; i < meshHeader.FaceGroupCount; i++)
    {
        FACE_GROUP       *pFace = nullptr;
        FACE_GROUP_HEADER faceGroupHeader;

        fread(&faceGroupHeader, sizeof(FACE_GROUP_HEADER), 1, fp);
        pFace = FACE_GROUP::Alloc(faceGroupHeader);
        fread(&pFace->pIndices, sizeof(uint32_t), faceGroupHeader.IndexCount, fp);

        ppFaces[i] = pFace;
    }

    m_header = meshHeader;
    m_pVertices = pVertices;
    m_ppFaceGroup = ppFaces;
}

BasicVertex *MeshObject::GetBasicVertices()
{
    if (m_header.Type == VERTEX_TYPE_BASIC)
    {
        return (BasicVertex *)m_pVertices;
    }
    return nullptr;
}

SkinnedVertex *MeshObject::GetSkinnedVertices()
{
    if (m_header.Type == VERTEX_TYPE_SKINNED)
    {
        return (SkinnedVertex *)m_pVertices;
    }
    return nullptr;
}

FACE_GROUP *MeshObject::GetFaceGroup(uint32_t index)
{
    if (index < 0 || index >= m_header.FaceGroupCount)
    {
        __debugbreak();
        return nullptr;
    }
    return m_ppFaceGroup[index];
}

bool MeshObject::IsInFrustum(const Matrix &worldMat, const BoundingFrustum &frustum) const
{
    /*Sphere collisionSphere = GetCollisionSphere();

    auto boundingResult = frustum.Contains(collisionSphere);

    if (boundingResult == DirectX::DISJOINT)
        return false;
    return true;*/
    return false;
}

void MeshObject::Render(IRenderer *pRnd, const Matrix &worldMat)
{
    // Debug
    /*m_pRenderer->RenderMeshObject(m_pCollisionSphereMesh, &worldMat, true);
    m_pRenderer->RenderMeshObject(m_pCollisionBoxMesh, &worldMat, true);*/

    m_pRenderer->RenderMeshObject(m_pMeshHandle, &worldMat);
}

MeshObject::~MeshObject() { Cleanup(); }
