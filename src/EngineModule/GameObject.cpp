#include "pch.h"

#include "GameEngine.h"
#include "Model.h"

#include "GameObject.h"

void GameObject::Cleanup()
{
    if (m_pModel)
    {
        m_pModel->Release();
        m_pModel = nullptr;
    }
}

void GameObject::Initialize(GameEngine *pGameEngine)
{
    m_pGameEngine = pGameEngine;
    m_pRenderer = pGameEngine->GetRenderer();
}

void GameObject::Run()
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

void GameObject::ApplyImpulseLinear(const Vector3 &impulse)
{
    if (m_invMass == 0.0f)
        return;
    m_linearVelocity += impulse * m_invMass;
}

BoundingOrientedBox GameObject::GetBoxInWorld()
{
    if (!m_pModel)
        return BoundingOrientedBox();

    BoundingOrientedBox worldBox;

    m_pModel->GetBoundingBox().Transform(worldBox, m_transform.GetMatrix());
    m_collisionBox = worldBox;

    return worldBox;
}

BoundingSphere GameObject::GetSphereInWorld()
{
    if (!m_pModel)
        return BoundingSphere();

    BoundingSphere worldSphere;
    m_pModel->GetBoundingSphere().Transform(worldSphere, m_transform.GetMatrix());

    m_collisionSphere = worldSphere;
    return worldSphere;
}

BOOL GameObject::IsIntersect(GameObject *pOther)
{
    if (m_collisionType == COLLISION_SHAPE_TYPE_NONE || pOther->m_collisionType == COLLISION_SHAPE_TYPE_NONE)
        return FALSE;

    if (m_collisionType == COLLISION_SHAPE_TYPE_BOX)
    {
        if (pOther->m_collisionType == COLLISION_SHAPE_TYPE_BOX)
        {
            return GetBoxInWorld().Intersects(pOther->GetBoxInWorld());
        }
        else
        {
            return GetBoxInWorld().Intersects(pOther->GetSphereInWorld());
        }
    }
    else
    {
        if (pOther->m_collisionType == COLLISION_SHAPE_TYPE_BOX)
        {
            return GetSphereInWorld().Intersects(pOther->GetBoxInWorld());
        }
        else
        {
            return GetSphereInWorld().Intersects(pOther->GetSphereInWorld());
        }
    }
}

void GameObject::SetPhysics(COLLISION_SHAPE_TYPE collisionType, float mass)
{
    m_collisionType = collisionType;
    m_invMass = (mass < 1e-4f) ? 0.0f : 1.0f / mass;
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
