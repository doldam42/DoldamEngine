#include "pch.h"

#include "GameEngine.h"
#include "GameObject.h"
#include "RenderModule.h"

#include "Model.h"

void Model::Cleanup()
{
    if (m_pMaterials)
    {
        delete[] m_pMaterials;
        m_pMaterials = nullptr;
    }
    if (m_ppObjs)
    {
        for (uint32_t i = 0; i < m_header.ObjectCount; i++)
        {
            delete m_ppObjs[i];
            m_ppObjs[i] = nullptr;
        }
        delete[] m_ppObjs;
        m_ppObjs = nullptr;
    }
}

void Model::Initialize(const Material *pInMaterial, int materialCount, BasicObject **ppInObjs, int objectCount)
{
    m_header.MaterialCount = materialCount;
    m_header.ObjectCount = objectCount;

    Material *pMaterials = new Material[materialCount];
    for (UINT i = 0; i < materialCount; i++)
    {
        pMaterials[i] = pInMaterial[i];
    }

    BasicObject **ppObjs = new BasicObject *[objectCount];
    for (UINT i = 0; i < objectCount; i++)
    {
        ppObjs[i] = ppInObjs[i];
    }

    m_pMaterials = pMaterials;
    m_ppObjs = ppObjs;
}

void Model::InitMeshHandles(IRenderer *pRenderer)
{
    for (uint32_t i = 0; i < m_header.ObjectCount; i++)
    {
        BasicObject *pObj = m_ppObjs[i];
        pObj->InitMeshHandle(pRenderer, m_pMaterials, m_basePath);
    }
}

void Model::SetBasePath(const WCHAR *basePath)
{
    memset(m_basePath, 0, sizeof(m_basePath));
    wcscpy_s(m_basePath, wcslen(basePath) + 1, basePath);
}

void Model::WriteFile(FILE *fp)
{
    fwrite(&m_header, sizeof(MODEL_HEADER), 1, fp);
    fwrite(m_pMaterials, sizeof(Material), m_header.MaterialCount, fp);
    for (uint32_t i = 0; i < m_header.ObjectCount; i++)
    {
        BasicObject *pObj = m_ppObjs[i];
        pObj->WriteFile(fp);
    }
}

void Model::ReadFile(FILE *fp)
{
    MODEL_HEADER  modelHeader;
    OBJECT_HEADER objHeader;
    fread(&modelHeader, sizeof(MODEL_HEADER), 1, fp);

    Material     *pMaterial = new Material[modelHeader.MaterialCount];
    BasicObject **ppObjs = new BasicObject *[modelHeader.ObjectCount];

    fread(pMaterial, sizeof(Material), modelHeader.MaterialCount, fp);

    for (uint32_t i = 0; i < modelHeader.ObjectCount; i++)
    {
        BasicObject *pObj = nullptr;
        fread(&objHeader, sizeof(OBJECT_HEADER), 1, fp);

        switch (objHeader.Type)
        {
        case OBJECT_TYPE_MESH: {
            MeshObject *pMesh = new MeshObject(objHeader);
            pObj = pMesh;
        }
        break;
        case OBJECT_TYPE_CHARACTER: {
            CharacterObject *pChar = new CharacterObject(objHeader);
            pObj = pChar;
        }
        break;

        default:
            __debugbreak();
            break;
        }
        pObj->ReadFile(fp);
        ppObjs[i] = pObj;
    }

    m_pMaterials = pMaterial;
    m_ppObjs = ppObjs;
    m_header = modelHeader;
}

void Model::UpdateAnimation(AnimationClip *pClip, int frame)
{
    for (uint32_t objIdx = 0; objIdx < m_header.ObjectCount; objIdx++)
    {
        BasicObject *pObj = m_ppObjs[objIdx];
        if (pObj->GetType() == OBJECT_TYPE_CHARACTER)
        {
            ((CharacterObject *)pObj)->UpdateAnimation(pClip, frame);
        }
    }
}

void Model::Render(IRenderer *pRenderer, GameObject *pGameObj)
{
    if (pGameObj)
    {
        Matrix worldMat = pGameObj->GetWorldMatrix();

        const Matrix &viewProj = g_pGame->GetCamera()->GetViewProjRow();

        Matrix finalMatrix = viewProj * worldMat;

        Matrix finalTransposedMatrix = finalMatrix.Transpose();

        Plane frustumPlanesFromMatrix[] = {
            Plane(-(Vector4(finalTransposedMatrix.m[3]) - Vector4(finalTransposedMatrix.m[1]))), // up
            Plane(-(Vector4(finalTransposedMatrix.m[3]) + Vector4(finalTransposedMatrix.m[1]))), // bottom
            Plane(-(Vector4(finalTransposedMatrix.m[3]) - Vector4(finalTransposedMatrix.m[0]))), // right
            Plane(-(Vector4(finalTransposedMatrix.m[3]) + Vector4(finalTransposedMatrix.m[0]))), // left
            Plane(-(Vector4(finalTransposedMatrix.m[3]) - Vector4(finalTransposedMatrix.m[2]))), // far
            Plane(-(Vector4(finalTransposedMatrix.m[3]) + Vector4(finalTransposedMatrix.m[2]))), // near
        };

        Frustum frustumFromMatrix(frustumPlanesFromMatrix);

        const BoundingFrustum &worldFrustum = g_pGame->GetFrustum();
        int                    culledMeshCount = 0;
        for (uint32_t objectCount = 0; objectCount < m_header.ObjectCount; objectCount++)
        {
            MeshObject *pObj = (MeshObject *)m_ppObjs[objectCount];
            // Box         boundBox = pObj->GetCollisionBox();
            Sphere      boundingSphere = pObj->GetCollisionSphere();
            auto        checkResult = frustumFromMatrix.CheckBound(boundingSphere);
            if (checkResult == BoundCheckResult::Outside)
            {
                g_pGame->m_culledObjectCountForDebug++;
            }
            else
            {
                pObj->Render(pRenderer, worldMat);
            }
        }
    }
}

Model::Model()
{
    m_LinkInGame.pItem = this;
    m_LinkInGame.pNext = nullptr;
    m_LinkInGame.pPrev = nullptr;
}

Model::~Model() { Cleanup(); }

HRESULT __stdcall Model::QueryInterface(REFIID riid, void **ppvObject) { return E_NOTIMPL; }

ULONG __stdcall Model::AddRef(void)
{
    ref_count++;
    if (ref_count > MAX_INSTANCE_COUNT)
    {
        __debugbreak();
    }
    return ref_count;
}

ULONG __stdcall Model::Release(void)
{
    ULONG newRefCount = --ref_count;
    if (newRefCount == 0)
    {
        delete this;
        return 0;
    }
    return newRefCount;
}
