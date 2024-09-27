#include "pch.h"

#include "GameObject.h"

#include "Model.h"

void Model::Cleanup()
{
    if (m_pMaterials)
    {
        delete[] m_pMaterials;
        m_pMaterials = nullptr;
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

void Model::Initialize(const Material *pInMaterial, int materialCount, void **ppInObjs, int objectCount)
{
    m_materialCount = materialCount;
    m_objectCount = objectCount;

    Material *pMaterials = new Material[materialCount];
    for (UINT i = 0; i < materialCount; i++)
    {
        pMaterials[i] = pInMaterial[i];
    }

    MeshObject **ppObjs = new MeshObject *[objectCount];
    for (UINT i = 0; i < objectCount; i++)
    {
        ppObjs[i] = reinterpret_cast<MeshObject *>(ppInObjs[i]);
    }

    m_pMaterials = pMaterials;
    m_ppMeshObjects = ppObjs;
}

void Model::InitMeshHandles(IRenderer *pRenderer)
{
    for (UINT i = 0; i < m_objectCount; i++)
    {
        m_ppMeshObjects[i]->InitMeshHandle(pRenderer, m_pMaterials, m_basePath);
    }
    m_pRenderer = pRenderer;
}

void Model::ReadFile(FILE *fp)
{
    fread(&m_objectCount, sizeof(UINT), 1, fp);
    fread(&m_materialCount, sizeof(UINT), 1, fp);

    m_ppMeshObjects = new MeshObject *[m_objectCount];
    m_pMaterials = new Material[m_materialCount];

    fread(m_pMaterials, sizeof(Material), m_materialCount, fp);
    for (int i = 0; i < m_objectCount; i++)
    {
        MeshObject *pMesh = new MeshObject;
        pMesh->ReadFile(fp);
        m_ppMeshObjects[i] = pMesh;
    }
}

void Model::WriteFile(FILE *fp)
{
    fwrite(&m_objectCount, sizeof(UINT), 1, fp);
    fwrite(&m_materialCount, sizeof(UINT), 1, fp);

    fwrite(m_pMaterials, sizeof(Material), m_materialCount, fp);
    for (int i = 0; i < m_objectCount; i++)
    {
        MeshObject *pMesh = m_ppMeshObjects[i];
        pMesh->WriteFile(fp);
    }
}

void Model::Render(GameObject *pGameObj) 
{ 
    const Matrix worldMat = pGameObj->GetWorldMatrix();
    for (UINT i = 0; i < m_objectCount; i++)
    {
        m_ppMeshObjects[i]->Render(m_pRenderer, &worldMat);
    }
}

void Model::SetBasePath(const WCHAR *basePath)
{
    memset(m_basePath, L'\0', sizeof(m_basePath));
    wcscpy_s(m_basePath, wcslen(basePath) + 1, basePath);
}

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

Model::~Model() { Cleanup(); }
