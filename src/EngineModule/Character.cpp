#include "pch.h"

#include "AnimationClip.h"
#include "Character.h"
#include "GameEngine.h"
#include "Model.h"

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

void Character::Initialize(GameEngine *pGameEngine, UINT maxClipCount)
{
    GameObject::Initialize(pGameEngine);
    m_maxClipCount = maxClipCount;
    m_ppAnimationClips = new AnimationClip *[maxClipCount];
}

void Character::Update(float dt)
{
    GameObject::Update(dt);

    AnimationClip *clip = m_ppAnimationClips[m_clipId];
    GetModel()->UpdateAnimation(clip, m_frameCount);
    m_frameCount++;
    m_frameCount %= 3600;
}

Character::~Character() { Cleanup(); }

void Character::InitPhysics(const Shape *pInShape, float mass, float elasticity)
{
    GameObject::InitPhysics(pInShape, mass, elasticity);
}

Vector3 Character::GetPosition() { return GameObject::GetPosition(); }

Vector3 Character::GetScale() { return GameObject::GetScale(); }

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

void Character::AddPosition(const Vector3 *pInDeltaPos) { GameObject::AddPosition(pInDeltaPos); }

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
