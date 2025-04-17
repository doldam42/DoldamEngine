#include "pch.h"

#include "CameraController.h"
#include "Client.h"
#include "InputManager.h"

#include "CharacterController.h"

void CharacterController::ApplyGravity(float dt) 
{
    constexpr float gravity = -9.8f;
    m_velocity.y += dt * gravity;
}

void CharacterController::UpdatePosition(float dt) 
{ 
    Vector3 curPos = m_pGameObject->GetPosition();
    Vector3 deltaPos = curPos + m_velocity * dt;
    Vector3 foot = curPos - Vector3(0.0f, m_height / 2, 0.0f);

    Vector3 dir = deltaPos - curPos;

    float length = dir.Length();

    dir.Normalize();
    Ray ray;
    ray.direction = dir;
    ray.position = foot;
    ray.tmax = length;

    ICollider *pHittedCollider = nullptr;
    float tHit;
    if (m_pPhysics->Raycast(ray, &tHit, &pHittedCollider))
    {
        deltaPos = curPos + dir * tHit * 0.99f;
        if (pHittedCollider != m_pCollider)
        {

        }
    }
}

void CharacterController::CheckGrounded() {}

BOOL CharacterController::Initialize(const Vector3 &startPosition, float height, float radius)
{
    IGameManager    *pGame = g_pClient->GetGameManager();
    IPhysicsManager *pPhysics = g_pClient->GetPhysics();

    IGameObject *pObj = pGame->CreateGameObject();
    ICollider   *pCollider = pPhysics->CreateEllpsoidCollider(pObj, height, radius);

    pObj->SetCollider(pCollider);
    pObj->SetPosition(startPosition.x, startPosition.y + 0.001f + height / 2, startPosition.z);

    m_pPhysics = pPhysics;

    m_pGameObject = pObj;
    m_pCollider = pCollider;

    m_height = height;
    m_radius = radius;

    return TRUE;
}
