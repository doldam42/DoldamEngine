#include "pch.h"

#include "CameraController.h"
#include "Client.h"
#include "InputManager.h"

#include "CollisionDemoController.h"

//REGISTER_CONTROLLER(CollisionDemoController)

BOOL CollisionDemoController::Start()
{
    CameraController *pCam = g_pClient->GetCameraController();
    IGameManager     *pGame = g_pClient->GetGameManager();
    IRenderer        *pRenderer = pGame->GetRenderer();

    m_pPlayer = pGame->CreateGameObject();
    m_pPlayer->SetPosition(0.0f, 2.f, 0.0f);
    pCam->SetFollowTarget(m_pPlayer);

    return TRUE;
}

void CollisionDemoController::Update(float dt)
{
    InputManager *pI = g_pClient->GetInputManager();
    IGameManager *pGame = g_pClient->GetGameManager();

    RayHit hit;
    BOOL   isGrounded = pGame->Raycast(m_pPlayer->GetPosition(), Vector3::Down, &hit, 0.1f);
    if (isGrounded)
    {
        jumpSpeed = (pI->IsKeyPressed(VK_SPACE, false)) ? 9.8 : 0.0f;
        speed = (pI->IsKeyPressed(VK_SHIFT)) ? SPEED * 2.0f : SPEED;
    }
    else
    {
        jumpSpeed -= 9.8f * dt;
    }

    const float dx = pI->GetXAxis();
    const float dz = pI->GetZAxis();

    Vector3 forward = m_pPlayer->GetForward();
    Vector3 right = Vector3::Up.Cross(forward);
    
    Vector3 deltaPos = (forward * dz + right * dx) * speed;
    deltaPos.y += jumpSpeed;
    deltaPos *= dt;
    m_pPlayer->AddPosition(deltaPos);
}

