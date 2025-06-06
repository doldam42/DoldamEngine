#include "pch.h"

#include "AnimationClip.h"
#include "GameManager.h"
#include "Model.h"

#include "Character.h"

void Character::Cleanup()
{
    if (m_ppAnimationClips)
    {
        for (int i = 0; i < m_clipCount; i++)
        {
            AnimationClip *pClip = m_ppAnimationClips[i];
            if (pClip)
            {
                pClip->Release();
                pClip = nullptr;
            }
        }
        delete[] m_ppAnimationClips;
    }
}

void Character::Initialize(GameManager *pGameEngine, UINT maxClipCount)
{
    GameObject::Initialize(pGameEngine);
    m_maxClipCount = maxClipCount;
    m_ppAnimationClips = new AnimationClip *[maxClipCount];
}

void Character::Render()
{
    if (m_isVisible && m_isActive && m_pModel)
    {
        AnimationClip *clip = m_ppAnimationClips[m_clipId];
        m_pModel->Render(this, m_ppMaterials, m_materialCount, clip, m_frameCount);
        m_frameCount++;
        m_frameCount %= 3600;
    }
}

Character::~Character() { Cleanup(); }

void Character::SetCollider(ICollider *pCollider) { GameObject::SetCollider(pCollider); }

void Character::SetRigidBody(IRigidBody *pBody) { GameObject::SetRigidBody(pBody); }

Vector3 Character::GetPosition() { return GameObject::GetPosition(); }

Vector3 Character::GetScale() { return GameObject::GetScale(); }

Vector3 Character::GetForward() { return GameObject::GetForward(); }

float Character::GetRotationX() { return GameObject::GetRotationX(); }

float Character::GetRotationY() { return GameObject::GetRotationY(); }

float Character::GetRotationZ() { return GameObject::GetRotationZ(); }

Quaternion Character::GetRotation() { return GameObject::GetRotation(); }

void Character::SetModel(IGameModel *pModel) { GameObject::SetModel(pModel); }

void Character::SetPosition(float x, float y, float z) { GameObject::SetPosition(x, y, z); }

void Character::SetScale(float x, float y, float z) { GameObject::SetScale(x, y, z); }

void Character::SetScale(float s) { GameObject::SetScale(s); }

void Character::SetRotationX(float rotX) { GameObject::SetRotationX(rotX); }

void Character::SetRotationY(float rotY) { GameObject::SetRotationY(rotY); }

void Character::SetRotationZ(float rotZ) { GameObject::SetRotationZ(rotZ); }

void Character::SetRotation(Quaternion q) { GameObject::SetRotation(q); }

void Character::SetActive(BOOL state) { GameObject::SetActive(state); }

void Character::AddPosition(Vector3 deltaPos) { GameObject::AddPosition(deltaPos); }

void Character::SetMaterials(IRenderMaterial **ppMaterials, const UINT numMaterials)
{
    GameObject::SetMaterials(ppMaterials, numMaterials);
}

IRenderMaterial *Character::GetMaterialAt(UINT index) { return GameObject::GetMaterialAt(index); }

size_t Character::GetID() { return GameObject::GetID(); }

Bounds Character::GetBounds() const { return GameObject::GetBounds(); }

bool Character::IntersectRay(const Ray &ray, float *hitt0, float *hitt1) const
{
    return GameObject::IntersectRay(ray, hitt0, hitt1);
}

bool Character::Intersect(const Bounds &b) const { return GameObject::Intersect(b); }

void Character::SetLayer(UINT layer) { GameObject::SetLayer(layer); }

UINT Character::GetLayer() { return GameObject::GetLayer(); }

ICollider *Character::GetCollider() const { return GameObject::GetCollider(); }

IRigidBody *Character::GetRigidBody() const { return GameObject::GetRigidBody(); }

void Character::InsertAnimation(IGameAnimation *pClip)
{
    m_ppAnimationClips[m_clipCount] = (AnimationClip *)pClip;
    m_clipCount++;
}

void Character::SetCurrentAnimationByName(const WCHAR *pAnimationName)
{
    for (int i = 0; i < m_clipCount; i++)
    {
        AnimationClip *pClip = m_ppAnimationClips[i];
        if (!wcscmp(pAnimationName, pClip->GetName()))
        {
            m_clipId = i;
            return;
        }
    }
    IGameAnimation *pClip = g_pGame->GetAnimationByName(pAnimationName);
    if (pClip != nullptr)
    {
        m_ppAnimationClips[m_clipCount] = (AnimationClip *)pClip;
        m_clipCount++;
    }
}
