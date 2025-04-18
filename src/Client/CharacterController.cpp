#include "pch.h"

#include "CameraController.h"
#include "Client.h"
#include "InputManager.h"

#include "CharacterController.h"

void CharacterController::ApplyGravity(float dt)
{
    constexpr float gravity = 9.8f;
    m_velocity.y -= dt * gravity;
}

void CharacterController::CheckGrounded()
{
    const Vector3 curPos = m_pGameObject->GetPosition();
    const Vector3 foot = curPos - Vector3(0.0f, m_height / 2 + 0.02f, 0.0f);

    Ray ray;
    ray.direction = Vector3::Down;
    ray.position = foot;
    ray.tmax = 0.01f;

    float      tHit;
    ICollider *pHitted = nullptr;

    m_isGrounded = m_pPhysics->Raycast(ray, &tHit, &pHitted);
}

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

void CharacterController::Update(float dt)
{
    CheckGrounded();
    if (m_isGrounded && m_velocity.y < 0.01f)
    {
        m_velocity.y = 0;
    }
    else
    {
        ApplyGravity(dt);
    }

    m_pGameObject->AddPosition(m_velocity * dt);
}

void CharacterController::Move(const Vector3 &dir)
{
    m_velocity.x = dir.x;
    m_velocity.z = dir.z;
}

void CharacterController::Stop()
{
    m_velocity.x = 0.0f;
    m_velocity.z = 0.0f;
}

void CharacterController::Jump(float jumpSpeed) { m_velocity.y = jumpSpeed; }
