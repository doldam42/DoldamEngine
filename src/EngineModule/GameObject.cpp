#include "pch.h"

#include "GameManager.h"
#include "Model.h"

#include "BoxCollider.h"
#include "SphereCollider.h"
#include "ConvexCollider.h"

#include "RigidBody.h"
#include "PhysicsManager.h"
#include "GameObject.h"

size_t GameObject::g_id = 0;

void GameObject::CleanupMaterials()
{
    if (m_ppMaterials)
    {
        for (int i = 0; i < m_materialCount; i++)
        {
            m_ppMaterials[i]->Release();
            m_ppMaterials[i] = nullptr;
        }
        delete[] m_ppMaterials;
        m_ppMaterials = nullptr;
    }
}

void GameObject::Cleanup()
{
    CleanupMaterials();
    if (m_pRigidBody)
    {
        m_pGame->GetPhysicsManager()->DeleteRigidBody(m_pRigidBody);
        m_pRigidBody = nullptr;
    }
    if (m_pModel)
    {
        m_pModel->Release();
        m_pModel = nullptr;
    }
}

void GameObject::Initialize(GameManager *pGameEngine)
{
    m_pGame = pGameEngine;
    m_pRenderer = pGameEngine->GetRenderer();
}

BOOL GameObject::InitBoxCollider(const Vector3 &center, const Vector3 &extent)
{
    BoxCollider *pBox = new BoxCollider;
    pBox->Initialize(this, center, extent);
    m_pCollider = pBox;

    return TRUE;
}

BOOL GameObject::InitSphereCollider(const Vector3 &center, const float radius)
{
    SphereCollider *pSphere = new SphereCollider;
    pSphere->Initialize(this, center, radius);
    m_pCollider = pSphere;

    return TRUE;
}

BOOL GameObject::InitConvexCollider()
{ 
    if (!m_pModel)
        return FALSE;

    // TODO: Reserve Points
    std::vector<Vector3> points; 
    UINT numObj = m_pModel->GetObjectCount();
    for (UINT i = 0; i < numObj; i++)
    {
        MeshObject* pObj = m_pModel->GetObjectByIdx(i);
        
        UINT numVertice = pObj->GetVertexCount();
        
        if (pObj->IsSkinned())
        {
            SkinnedVertex *pVertice = pObj->GetSkinnedVertices();
            for (int j = 0; j < numVertice; j++)
            {
                points.push_back(pVertice[j].position);
            }
        }
        else
        {
            BasicVertex *pVertice = pObj->GetBasicVertices();
            for (int j = 0; j < numVertice; j++)
            {
                points.push_back(pVertice[j].position);
            }
        }
    }

    ConvexCollider *pConvex = new ConvexCollider;

    m_pCollider = pConvex;

    pConvex->Initialize(this, points.data(), points.size());

    return TRUE;
}

BOOL GameObject::InitRigidBody(float mass, float elasticity, float friction, BOOL useGravity,
                               BOOL isKinematic)
{
    if (!m_pCollider)
        return FALSE;

    PhysicsManager *pPhysics = m_pGame->GetPhysicsManager();
    m_pRigidBody = pPhysics->CreateRigidBody(this, m_pCollider, mass, elasticity, friction, useGravity, isKinematic);
    return TRUE;
}

void GameObject::Update(float dt)
{
    if (m_IsUpdated)
    {
        m_worldMatrix = m_transform.GetMatrix();

        if (m_pCollider)
        {
            m_pCollider->Update();
        }

        m_IsUpdated = false;
    }
}

void GameObject::Render()
{
    if (m_pModel && m_isVisible)
    {
        m_pModel->Render(this, m_ppMaterials, m_materialCount);
    }
}

void GameObject::SetModel(IGameModel *pModel)
{
    if (m_pModel)
    {
        m_pModel->Release();
        m_pModel = nullptr;
    }

    m_pModel = static_cast<Model *>(pModel);
}

void GameObject::SetPosition(float x, float y, float z)
{
    m_transform.SetPosition(Vector3(x, y, z));
    m_IsUpdated = true;
}

void GameObject::SetScale(float x, float y, float z)
{
    m_transform.SetScale(Vector3(x, y, z));
    m_IsUpdated = true;
}

void GameObject::SetScale(float s)
{
    m_transform.SetScale(Vector3(s));
    m_IsUpdated = true;
}

void GameObject::SetRotationY(float rotY)
{
    m_transform.SetRotation(Quaternion::CreateFromYawPitchRoll(rotY, 0, 0));
    m_IsUpdated = true;
}

GameObject::GameObject()
{
    m_LinkInGame.pItem = this;
    m_LinkInGame.pNext = nullptr;
    m_LinkInGame.pPrev = nullptr;

    m_LinkInWorld.pItem = this;
    m_LinkInWorld.pNext = nullptr;
    m_LinkInWorld.pPrev = nullptr;

    m_worldMatrix = Matrix::Identity;

    m_id = InterlockedIncrement(&g_id);
}

GameObject::~GameObject() { Cleanup(); }

void GameObject::SetRotationX(float rotX)
{
    m_transform.SetRotation(Quaternion::CreateFromYawPitchRoll(0, rotX, 0));
    m_IsUpdated = true;
}

void GameObject::SetRotationZ(float rotZ)
{
    m_transform.SetRotation(Quaternion::CreateFromYawPitchRoll(0, 0, rotZ));
    m_IsUpdated = true;
}

void GameObject::SetRotation(Quaternion q)
{
    m_transform.SetRotation(q);
    m_IsUpdated = true;
}

void GameObject::AddPosition(Vector3 deltaPos)
{
    m_transform.AddPosition(deltaPos);
    m_IsUpdated = true;
}

// TODO: IMaterialHandle에 IUnknown 붙이기
void GameObject::SetMaterials(IRenderMaterial **ppMaterials, const UINT numMaterials)
{
    if (!m_ppMaterials)
    {
        m_ppMaterials = new IRenderMaterial *[numMaterials];
        ZeroMemory(m_ppMaterials, sizeof(IRenderMaterial *) * numMaterials);
        m_materialCount = numMaterials;
    }

    assert(m_materialCount == numMaterials);

    for (UINT i = 0; i < numMaterials; i++)
    {
        if (m_ppMaterials[i])
        {
            m_ppMaterials[i]->Release();
            m_ppMaterials[i] = nullptr;
        }
        m_ppMaterials[i] = ppMaterials[i];
    }
}

IRenderMaterial *GameObject::GetMaterialAt(UINT index)
{
    if (!m_ppMaterials)
    {
        const UINT      materialCount = m_pModel->GetMaterialCount();
        const Material *pMaterials = m_pModel->GetMaterials();
        m_ppMaterials = new IRenderMaterial *[materialCount];
        for (UINT i = 0; i < materialCount; i++)
        {
            m_ppMaterials[i] = m_pRenderer->CreateMaterialHandle(pMaterials + i);
        }
        m_materialCount = materialCount;
    }
    return m_ppMaterials[index];
}

Bounds GameObject::GetBounds() const
{
    Bounds box;
    m_pModel->GetBoundingBox().Transform(&box, m_transform.GetMatrix());
    return box;
}

bool GameObject::IntersectRay(const Ray &ray, float *hitt0, float *hitt1) const
{
    if (!m_pCollider)
    {
        Bounds b = GetBounds();
        return b.IntersectP(ray, hitt0, hitt1);
    }
    return m_pCollider->IntersectRay(ray, hitt0, hitt1);
}

bool GameObject::Intersect(const Bounds& b) const
{
    if (!m_pCollider)
    {
        return GetBounds().DoesIntersect(b);
    }
    return m_pCollider->Intersect(b);
}
