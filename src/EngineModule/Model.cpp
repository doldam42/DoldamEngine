#include "pch.h"

#include "AnimationClip.h"
#include "GameManager.h"
#include "GameObject.h"
#include "GeometryGenerator.h"

#include "Model.h"

size_t Model::g_id = 0;

void Model::Cleanup()
{
    if (m_pBoundingBoxMesh)
    {
        m_pBoundingBoxMesh->Release();
        m_pBoundingBoxMesh = nullptr;
    }
    if (m_pBoneMatrices)
    {
        delete[] m_pBoneMatrices;
        m_pBoneMatrices = nullptr;
    }
    if (m_pMaterials)
    {
        delete[] m_pMaterials;
        m_pMaterials = nullptr;
    }
    if (m_pJoints)
    {
        delete[] m_pJoints;
        m_pJoints = nullptr;
    }
    if (m_ppMaterials)
    {
        for (int i = 0; i < m_materialCount; i++)
        {
            m_ppMaterials[i]->Release();
            m_ppMaterials[i] = nullptr;
        }
        delete[] m_ppMaterials;
    }
    if (m_ppMeshObjects)
    {
        for (UINT i = 0; i < m_objectCount; i++)
        {
            delete m_ppMeshObjects[i];
            m_ppMeshObjects[i] = nullptr;
        }
        delete[] m_ppMeshObjects;
        m_ppMeshObjects = nullptr;
    }
}

void Model::Initialize(const Material *pInMaterial, int materialCount, IGameMesh **ppInObjs, int objectCount,
                       Joint *pInJoint, int jointCount)
{
    m_materialCount = materialCount;
    m_objectCount = objectCount;
    m_jointCount = jointCount;

    if (pInMaterial)
    {
        Material *pMaterials = new Material[materialCount];
        for (UINT i = 0; i < materialCount; i++)
        {
            pMaterials[i] = pInMaterial[i];
        }
        m_pMaterials = pMaterials;
    }

    if (ppInObjs)
    {
        MeshObject **ppObjs = new MeshObject *[objectCount];
        for (UINT i = 0; i < objectCount; i++)
        {
            ppObjs[i] = dynamic_cast<MeshObject *>(ppInObjs[i]);
        }
        m_ppMeshObjects = ppObjs;
    }

    if (pInJoint)
    {
        Joint *pJoints = new Joint[jointCount];
        for (UINT i = 0; i < jointCount; i++)
        {
            pJoints[i] = pInJoint[i];
        }
        m_pJoints = pJoints;
        m_pBoneMatrices = new Matrix[jointCount];
    }
}

void Model::InitRenderComponents(IRenderer *pRenderer)
{
    for (UINT i = 0; i < m_materialCount; i++)
    {
        wcscpy_s(m_pMaterials[i].basePath, m_basePath);
    }

    m_ppMaterials = new IRenderMaterial *[m_materialCount];
    for (UINT i = 0; i < m_materialCount; i++)
    {
        m_ppMaterials[i] = pRenderer->CreateMaterialHandle(m_pMaterials + i);
    }

    for (UINT i = 0; i < m_objectCount; i++)
    {
        m_ppMeshObjects[i]->InitRenderComponent(pRenderer, m_pJoints, m_jointCount, m_ppMaterials, m_materialCount);
    }

    m_pRenderer = pRenderer;
    InitBoundary();
}

void Model::InitBoundary()
{
    Vector3 minCorner(FLT_MAX);
    Vector3 maxCorner(FLT_MIN);

    for (int i = 0; i < m_objectCount; i++)
    {
        MeshObject *pObj = GetObjectByIdx(i);

        UINT vertexCount = pObj->GetVertexCount();
        if (vertexCount == 0)
            continue;

        if (pObj->IsSkinned())
        {
            SkinnedVertex *pVertice = pObj->GetSkinnedVertices();
            for (int i = 0; i < vertexCount; i++)
            {
                minCorner = Vector3::Min(minCorner, pVertice[i].position);
                maxCorner = Vector3::Max(maxCorner, pVertice[i].position);
            }
        }
        else
        {
            BasicVertex *pVertice = pObj->GetBasicVertices();
            for (int i = 0; i < vertexCount; i++)
            {
                minCorner = Vector3::Min(minCorner, pVertice[i].position);
                maxCorner = Vector3::Max(maxCorner, pVertice[i].position);
            }
        }
    }

    minCorner -= Vector3(1e-3f);
    maxCorner += Vector3(1e-3f);
    m_boundingBox = {minCorner, maxCorner};

    Vector3 center = m_boundingBox.Center();
    Vector3 extents = m_boundingBox.Extends();
    m_pBoundingBoxMesh = m_pRenderer->CreateWireBoxMesh(center, extents);
}

void Model::ReadFile(const char *filename)
{
    FILE *fp = nullptr;
    fopen_s(&fp, filename, "rb");
    if (!fp)
        __debugbreak();

    fread(&m_objectCount, sizeof(UINT), 1, fp);
    fread(&m_materialCount, sizeof(UINT), 1, fp);
    fread(&m_jointCount, sizeof(UINT), 1, fp);

    if (m_jointCount != 0)
    {
        m_pJoints = new Joint[m_jointCount];
    }
    if (m_materialCount != 0)
    {
        m_pMaterials = new Material[m_materialCount];
    }
    if (m_objectCount != 0)
    {
        m_ppMeshObjects = new MeshObject *[m_objectCount];
    }

    fread(m_pMaterials, sizeof(Material), (size_t)m_materialCount, fp);
    for (int i = 0; i < m_objectCount; i++)
    {
        MeshObject *pMesh = new MeshObject;
        pMesh->ReadFile(fp);
        m_ppMeshObjects[i] = pMesh;
    }
    fread(m_pJoints, sizeof(Joint), (size_t)m_jointCount, fp);

    // bone matrices는 File IO 하지 않음
    m_pBoneMatrices = new Matrix[m_jointCount];

    fclose(fp);
}

void Model::WriteFile(const char *filename)
{
    FILE *fp = nullptr;
    fopen_s(&fp, filename, "wb");
    if (!fp)
        __debugbreak();

    fwrite(&m_objectCount, sizeof(UINT), 1, fp);
    fwrite(&m_materialCount, sizeof(UINT), 1, fp);
    fwrite(&m_jointCount, sizeof(UINT), 1, fp);

    fwrite(m_pMaterials, sizeof(Material), (size_t)m_materialCount, fp);
    for (int i = 0; i < m_objectCount; i++)
    {
        MeshObject *pMesh = m_ppMeshObjects[i];
        pMesh->WriteFile(fp);
    }
    fwrite(m_pJoints, sizeof(Joint), (size_t)m_jointCount, fp);

    fclose(fp);
}

void Model::UpdateAnimation(AnimationClip *pClip, int frameCount)
{
    for (uint32_t boneId = 0; boneId < m_jointCount; boneId++)
    {
        Joint    *pJoint = m_pJoints + boneId;
        Keyframe *pKeyframe = pClip->GetKeyframeByIdx(boneId);

        const int    parentIdx = pJoint->parentIndex;
        const Matrix parentMatrix = parentIdx >= 0 ? m_pBoneMatrices[parentIdx] : Matrix::Identity;

        int    numKeys = pKeyframe->NumKeys;
        Matrix TM = numKeys > 0 ? pKeyframe->pKeys[frameCount % numKeys] : Matrix::Identity;

        m_pBoneMatrices[boneId] = TM * parentMatrix;
    }
    for (uint32_t boneId = 0; boneId < m_jointCount; boneId++)
    {
        Matrix &globalBindPoseInverse = m_pJoints[boneId].globalBindposeInverse;
        m_pBoneMatrices[boneId] =
            m_defaultTransform.Invert() * globalBindPoseInverse * m_pBoneMatrices[boneId] * m_defaultTransform;
    }
}

void Model::Render(GameObject *pGameObj, IRenderMaterial **ppMaterials, UINT numMaterials, AnimationClip *pClip,
                   int frameCount)
{
    const Matrix &worldMat = pGameObj->GetWorldMatrix();

    Keyframe **ppKeyframes = !pClip ? nullptr : pClip->GetKeyframes();
    if (!ppMaterials)
    {
        for (UINT i = 0; i < m_objectCount; i++)
        {
            m_ppMeshObjects[i]->Render(m_pRenderer, &worldMat, ppKeyframes, frameCount);
        }
    }
    else
    {
        for (UINT i = 0; i < m_objectCount; i++)
        {
            m_ppMeshObjects[i]->RenderWithMaterial(m_pRenderer, &worldMat, ppKeyframes, frameCount, ppMaterials,
                                                   numMaterials);
        }
    }
    // Render Bounding Box
    //m_pRenderer->RenderMeshObject(m_pBoundingBoxMesh, &worldMat, nullptr, 0, true);
}

void Model::SetBasePath(const WCHAR *basePath)
{
    memset(m_basePath, L'\0', sizeof(m_basePath));
    wcscpy_s(m_basePath, basePath);
}

void Model::SetMaterials(IRenderMaterial **ppMaterials, const UINT numMaterials) 
{ 
    if (!m_ppMaterials)
    {
        m_ppMaterials = new IRenderMaterial *[numMaterials];
        m_materialCount = numMaterials;
        ZeroMemory(m_ppMaterials, sizeof(IRenderMaterial *) * numMaterials);
    }

    assert(m_materialCount == numMaterials);
    for (int i = 0; i < numMaterials; i++)
    {
        if (m_ppMaterials[i])
        {
            m_ppMaterials[i]->Release();
            m_ppMaterials[i] = nullptr;
        }
        m_ppMaterials[i] = ppMaterials[i];
    }
}

HRESULT __stdcall Model::QueryInterface(REFIID riid, void **ppvObject) { return E_NOTIMPL; }

ULONG __stdcall Model::AddRef(void)
{
    ref_count++;
    /*if (ref_count > MAX_INSTANCE_COUNT)
    {
        __debugbreak();
    }*/
    return ref_count;
}

ULONG __stdcall Model::Release(void)
{
    ULONG newRefCount = --ref_count;
    if (newRefCount == 0)
    {
        g_pGame->DeleteModel(this);
    }
    return newRefCount;
}

Model::Model() : m_boundingBox()
{
    m_LinkInGame.pPrev = nullptr;
    m_LinkInGame.pNext = nullptr;
    m_LinkInGame.pItem = this;

    m_id = InterlockedIncrement(&g_id);
}

Model::~Model() { Cleanup(); }
