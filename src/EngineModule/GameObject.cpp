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

void GameObject::ResolveContact(GameObject *pA, GameObject *pB, const Contact &contact)
{
    const float invMassA = pA->m_invMass;
    const float invMassB = pB->m_invMass;

    const float elasticityA = pA->m_elasticity;
    const float elasticityB = pB->m_elasticity;
    const float elasticity = elasticityA * elasticityB;

    // Calculate the collision impulse
    const Vector3 &n = contact.normal;
    const Vector3  vab = pA->m_linearVelocity - pB->m_linearVelocity;

    const float    impulseJ = -(1.0f + elasticity) * vab.Dot(n) / (invMassA + invMassB);
    const Vector3  vectorImpulseJ = n * impulseJ;

    pA->ApplyImpulseLinear(vectorImpulseJ);
    pB->ApplyImpulseLinear(-vectorImpulseJ);

    const float tA = pA->m_invMass / (pA->m_invMass + pB->m_invMass);
    const float tB = pB->m_invMass / (pA->m_invMass + pB->m_invMass);

    const Vector3 ds = contact.worldSpaceA - contact.worldSpaceB;

    Vector3 posA = pA->GetPosition() + ds * tA;
    Vector3 posB = pB->GetPosition() - ds * tB;

    pA->SetPosition(posA.x, posA.y, posA.z);
    pB->SetPosition(posB.x, posB.y, posB.z);
}

void GameObject::ApplyImpulseLinear(const Vector3 &impulse)
{
    if (m_invMass == 0.0f)
        return;
    m_linearVelocity += impulse * m_invMass;
}

void GameObject::GetBoxInWorld(Box *pOutBox) const
{
    m_pModel->GetBoundingBox().LocalToWorld(pOutBox, m_transform.GetMatrix());
}

void GameObject::GetSphereInWorld(Sphere *pOutSphere) const
{
    m_pModel->GetBoundingSphere().LocalToWorld(pOutSphere, m_transform.GetMatrix());
}

BOOL GameObject::Intersect(const GameObject *pOther, Contact *pOutContact) const
{
    if (m_collisionShapeType == SHAPE_TYPE_NONE || pOther->m_collisionShapeType == SHAPE_TYPE_NONE)
        return FALSE;

    if (m_collisionShapeType == SHAPE_TYPE_BOX)
    {
        Box cur;
        GetBoxInWorld(&cur);
        if (pOther->m_collisionShapeType == SHAPE_TYPE_BOX)
        {
            Box other;
            pOther->GetBoxInWorld(&other);
            return cur.Intersect(other);
        }
        else
        {
            Sphere other;
            pOther->GetSphereInWorld(&other);
            return CheckCollision(cur, other, pOutContact);
        }
    }
    else
    {
        Sphere cur;
        GetSphereInWorld(&cur);
        if (pOther->m_collisionShapeType == SHAPE_TYPE_BOX)
        {
            Box other;
            pOther->GetBoxInWorld(&other);
            return CheckCollision(other, cur, pOutContact);
        }
        else
        {
            Sphere other;
            pOther->GetSphereInWorld(&other);
            return CheckCollision(cur, other, pOutContact);
        }
    }
}

void GameObject::SetPhysics(SHAPE_TYPE collisionType, float mass, float elasticity)
{
    m_collisionShapeType = collisionType;
    m_invMass = (mass < 1e-4f) ? 0.0f : 1.0f / mass;
    m_elasticity = elasticity;
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
