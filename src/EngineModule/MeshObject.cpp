#include "pch.h"

#include "MeshObject.h"

void MeshObject::Cleanup()
{
    if (m_pBasicVertices)
    {
        delete[] m_pBasicVertices;
        m_pBasicVertices = nullptr;
    }

    if (m_pSkinnedVertices)
    {
        delete[] m_pSkinnedVertices;
        m_pSkinnedVertices = nullptr;
    }

    if (m_pFaceGroups)
    {
        for (UINT i = 0; i < m_faceGroupCount; i++)
        {
            if (m_pFaceGroups[i].pIndices)
            {
                delete[] m_pFaceGroups[i].pIndices;
                m_pFaceGroups[i].pIndices = nullptr;
            }
        }
        delete[] m_pFaceGroups;
        m_pFaceGroups = nullptr;
    }

    if (m_pMeshHandle)
    {
        m_pMeshHandle->Release();
        m_pMeshHandle = nullptr;
    }
}

BOOL MeshObject::Initialize(MESH_TYPE meshType)
{
    m_meshType = meshType;
    return TRUE;
}

BOOL MeshObject::Initialize(const WCHAR *name, const Transform *pLocalTransform, int parentIndex,
                            int childCount, MESH_TYPE meshType)
{
    BaseObject::Initialize(name, pLocalTransform, parentIndex, childCount);
    m_meshType = meshType;
    return TRUE;
}

BOOL MeshObject::InitRenderComponent(IRenderer *pRnd, const Material *pMaterials, const WCHAR *basePath)
{
    if (IsSkinned())
    {
        m_pMeshHandle = pRnd->CreateSkinnedObject();
        m_pMeshHandle->BeginCreateMesh(m_pSkinnedVertices, m_vertexCount, m_faceGroupCount);
    }
    else
    {
        m_pMeshHandle = pRnd->CreateMeshObject();
        m_pMeshHandle->BeginCreateMesh(m_pBasicVertices, m_vertexCount, m_faceGroupCount);
    }

    for (int i = 0; i < m_faceGroupCount; i++)
    {
        FaceGroup *pFace = m_pFaceGroups + i;

        m_pMeshHandle->InsertFaceGroup(pFace->pIndices, pFace->numTriangles, pMaterials + pFace->materialIndex, basePath);
    }
    m_pMeshHandle->EndCreateMesh();
    return TRUE;
}

void MeshObject::BeginCreateMesh(const void *pVertices, UINT numVertices, UINT numFaceGroup)
{
    switch (m_meshType)
    {
    case MESH_TYPE_DEFAULT:
        m_pBasicVertices = new BasicVertex[numVertices];
        memcpy(m_pBasicVertices, pVertices, sizeof(BasicVertex) * numVertices);
        break;
    case MESH_TYPE_SKINNED:
        m_pSkinnedVertices = new SkinnedVertex[numVertices];
        memcpy(m_pSkinnedVertices, pVertices, sizeof(SkinnedVertex) * numVertices);
        break;
    default:
        __debugbreak();
        break;
    }

    m_pFaceGroups = new FaceGroup[numFaceGroup];

    m_vertexCount = numVertices;
    m_maxFaceGroupCount = numFaceGroup;
}

void MeshObject::InsertFaceGroup(const UINT *pIndices, UINT numTriangles, int materialIndex)
{
    FaceGroup *pFace = m_pFaceGroups + m_faceGroupCount;
    pFace->pIndices = new UINT[numTriangles * 3];
    memcpy(pFace->pIndices, pIndices, sizeof(UINT) * numTriangles * 3);

    pFace->numTriangles = numTriangles;
    pFace->materialIndex = materialIndex;

    m_faceGroupCount++;
}

void MeshObject::EndCreateMesh() {}

BOOL MeshObject::UpdateMaterial(IRenderMaterial *pMaterial, UINT faceGroupIndex)
{
    return m_pMeshHandle->UpdateMaterial(pMaterial, faceGroupIndex);
}

void MeshObject::ReadFile(FILE *fp)
{
    BaseObject::ReadFile(fp);

    fread(&m_meshType, sizeof(MESH_TYPE), 1, fp);
    fread(&m_vertexCount, sizeof(UINT), 1, fp);
    fread(&m_faceGroupCount, sizeof(UINT), 1, fp);

    if (IsSkinned())
    {
        m_pSkinnedVertices = new SkinnedVertex[m_vertexCount];
        fread(m_pSkinnedVertices, sizeof(SkinnedVertex), m_vertexCount, fp);
    }
    else
    {
        m_pBasicVertices = new BasicVertex[m_vertexCount];
        fread(m_pBasicVertices, sizeof(BasicVertex), m_vertexCount, fp);
    }

    m_pFaceGroups = new FaceGroup[m_faceGroupCount];
    for (int i = 0; i < m_faceGroupCount; i++)
    {
        FaceGroup *pFace = m_pFaceGroups + i;
        fread(&pFace->numTriangles, sizeof(UINT), 1, fp);
        fread(&pFace->materialIndex, sizeof(int), 1, fp);

        pFace->pIndices = new UINT[pFace->numTriangles * 3];
        fread(pFace->pIndices, sizeof(UINT), pFace->numTriangles * 3, fp);
    }
}

void MeshObject::WriteFile(FILE *fp)
{
    BaseObject::WriteFile(fp);

    fwrite(&m_meshType, sizeof(MESH_TYPE), 1, fp);
    fwrite(&m_vertexCount, sizeof(UINT), 1, fp);
    fwrite(&m_faceGroupCount, sizeof(UINT), 1, fp);

    if (IsSkinned())
    {
        fwrite(m_pSkinnedVertices, sizeof(SkinnedVertex), m_vertexCount, fp);
    }
    else
    {
        fwrite(m_pBasicVertices, sizeof(BasicVertex), m_vertexCount, fp);
    }

    for (int i = 0; i < m_faceGroupCount; i++)
    {
        FaceGroup *pFace = m_pFaceGroups + i;

        fwrite(&pFace->numTriangles, sizeof(UINT), 1, fp);
        fwrite(&pFace->materialIndex, sizeof(int), 1, fp);

        fwrite(pFace->pIndices, sizeof(UINT), pFace->numTriangles * 3, fp);
    }
}

void MeshObject::Render(IRenderer *pRnd, const Matrix *pWorldMat, const Matrix *pBoneMatrices, const UINT numJoints)
{
    Matrix finalMatrix = GetLocalTransform()->LocalToWorld(*pWorldMat).GetMatrix();
    switch (m_meshType)
    {
    case MESH_TYPE_DEFAULT:
        pRnd->RenderMeshObject(m_pMeshHandle, &finalMatrix);
        break;
    case MESH_TYPE_SKINNED:
        pRnd->RenderCharacterObject(m_pMeshHandle, &finalMatrix, pBoneMatrices, numJoints);
        break;
    default:
#ifdef _DEBUG
        __debugbreak();
#endif // _DEBUG
        break;
    }
}

void MeshObject::RenderWithMaterials(IRenderer *pRnd, const Matrix *pWorldMat, const Matrix *pBoneMatrices,
                                     const UINT numJoints, IRenderMaterial **ppMaterials, const UINT numMaterial)
{
    Matrix finalMatrix = GetLocalTransform()->LocalToWorld(*pWorldMat).GetMatrix();
    switch (m_meshType)
    {
    case MESH_TYPE_DEFAULT:
        pRnd->RenderMeshObjectWithMaterials(m_pMeshHandle, &finalMatrix, ppMaterials, numMaterial);
        break;
    case MESH_TYPE_SKINNED:
        pRnd->RenderCharacterObjectWithMaterials(m_pMeshHandle, &finalMatrix, pBoneMatrices, numJoints, ppMaterials,
                                                 numMaterial);
        break;
    default:
#ifdef _DEBUG
        __debugbreak();
#endif // _DEBUG
        break;
    }
}

MeshObject::~MeshObject() { Cleanup(); }
