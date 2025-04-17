#include "pch.h"

#include "CameraController.h"
#include "Client.h"
#include "InputManager.h"
#include "ControllerRegistry.h"
#include "CharacterController.h"

REGISTER_CONTROLLER(CharacterController)

BOOL CharacterController::Start()
{
    IGameManager     *pGame = g_pClient->GetGameManager();
    IPhysicsManager  *pPhysics = g_pClient->GetPhysics();
    CameraController *pCam = g_pClient->GetCameraController();

    const Vector3 pos(1.0f, 5.0f, -5.0f);
    m_pPlayer = pGame->CreateGameObject();
    m_pPlayer->SetPosition(pos.x, pos.y, pos.z);

    ICollider *pCollider = pPhysics->CreateEllpsoidCollider(m_pPlayer, 2.0f, 1.0f);
    m_pPlayer->SetCollider(pCollider);

    pCam->SetFollowTarget(m_pPlayer, Vector3(0.0f, 2.0f, -3.0f));

    return TRUE;
}

void CharacterController::Update(float dt)
{
    InputManager *pI = g_pClient->GetInputManager();
    IGameManager *pGame = g_pClient->GetGameManager();

    Vector3 pos = m_pPlayer->GetPosition();
    
    BOOL isGrounded = m_pPlayer->GetCollider()->IsCollisionEnter() || m_pPlayer->GetCollider()->IsCollisionStay();
    // RayHit hit;
    //BOOL   isGrounded = pGame->Raycast(pos, Vector3::Down, &hit, 0.01f);
    if (isGrounded)
    {
        /*if (hit.tHit < 0.0f)
            pos.y -= hit.tHit * dt;*/
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

    pos += (right * dx + forward * dz) * speed * dt;
    pos.y += jumpSpeed * dt;
    m_pPlayer->SetPosition(pos.x, pos.y, pos.z);
}
