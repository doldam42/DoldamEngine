#include "pch.h"

#include "GameManager.h"
#include "Model.h"
#include "PhysicsComponent.h"

#include "GameObject.h"

void GameObject::Cleanup()
{
    if (m_pModel)
    {
        m_pModel->Release();
        m_pModel = nullptr;
    }
    if (m_pPhysicsComponent)
    {
        delete m_pPhysicsComponent;
        m_pPhysicsComponent = nullptr;
    }
}

void GameObject::Initialize(GameManager *pGameEngine)
{
    m_pGameEngine = pGameEngine;
    m_pRenderer = pGameEngine->GetRenderer();
}

void GameObject::InitPhysics(const Shape *pInShape, float mass, float elasticity, float friction)
{
    m_pPhysicsComponent = new PhysicsComponent;
    m_pPhysicsComponent->Initialize(this, pInShape, mass, elasticity, friction);
}

void GameObject::Update(float dt)
{
    if (m_IsUpdated)
    {
        m_worldMatrix = m_transform.GetMatrix();
        m_IsUpdated = false;
    }
}

void GameObject::Render()
{
    if (m_pModel)
    {
        m_pModel->Render(this);
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

void GameObject::SetRotation(const Quaternion *pInQuaternion)
{
    m_transform.SetRotation(*pInQuaternion);
    m_IsUpdated = true;
}

void GameObject::AddPosition(const Vector3 *pInDeltaPos)
{
    m_transform.AddPosition(*pInDeltaPos);
    m_IsUpdated = true;
}

Bounds GameObject::GetBounds() const
{
    if (!m_pPhysicsComponent)
        nullptr;
    return m_pPhysicsComponent->GetBounds();
}
