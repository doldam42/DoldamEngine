#include "pch.h"

#include "CameraController.h"
#include "Client.h"
#include "ControllerRegistry.h"
#include "InputManager.h"

#include "PlayerController.h"

REGISTER_CONTROLLER(PlayerController)

BOOL PlayerController::Start()
{
    IGameManager     *pGame = g_pClient->GetGameManager();
    CameraController *pCam = g_pClient->GetCameraController();

    m_pPlayer = pGame->CreateGameObject();
    m_pPlayer->SetPosition(0.0f, 2.f, 0.0f);

    m_pPlayerBody = g_pClient->GetPhysics()->CreateCharacterBody(Vector3(0.0f, 2.0f, 0.0f), 0.5f, 2.0f);
    m_pPlayerBody->SetJumpSpeed(20.0f);

    pCam->SetFollowTarget(m_pPlayer, Vector3(0.0f, 2.0f, -3.0f));

    return TRUE;
}

void PlayerController::Update(float dt)
{
    InputManager *pI = g_pClient->GetInputManager();
    IGameManager *pGame = g_pClient->GetGameManager();

    if (pI->IsKeyPressed(VK_LBUTTON, false))
    {
        Vector3 rayDir = pGame->GetCameraLookTo();
        Vector3 rayPos = pGame->GetCameraPos();

        RayHit hit;
        if (pGame->Raycast(rayPos, rayDir, &hit))
        {
            IGameObject     *pHitted = hit.pHitted;
            IRenderMaterial *pMaterial = pHitted->GetMaterialAt(0);
            pMaterial->UpdateEmissive(Vector3(1.0f, 0.0f, 0.0f));
        }
    }
    else if (pI->IsKeyPressed(VK_RBUTTON, false))
    {
        Vector3 rayDir = pGame->GetCameraLookTo();
        Vector3 rayPos = pGame->GetCameraPos();

        RayHit hit;
        if (pGame->Raycast(rayPos, rayDir, &hit))
        {
            IGameObject     *pHitted = hit.pHitted;
            IRenderMaterial *pMaterial = pHitted->GetMaterialAt(0);
            pMaterial->UpdateEmissive(Vector3(0.0f, 0.0f, 0.0f));
        }
    }

    BOOL isGround = m_pPlayerBody->OnGround();

    speed = (pI->IsKeyPressed(VK_SHIFT)) ? SPEED * 2.0f : SPEED;

    if (isGround && pI->IsKeyPressed(VK_SPACE, false))
    {
        m_pPlayerBody->Jump();
    }
    else
    {
        const float dx = pI->GetXAxis();
        const float dz = pI->GetZAxis();

        Vector3 forward = m_pPlayer->GetForward();
        Vector3 right = Vector3::Up.Cross(forward);

        m_pPlayerBody->Move((right * dx + forward * dz) * speed * dt);
    }
    m_pPlayerBody->Update(m_pPlayer);
    //RayHit hit;
    //BOOL   isGrounded = pGame->Raycast(pos, Vector3::Down, &hit, 0.01f);
    //if (isGrounded)
    //{
    //    if (hit.tHit < 0.0f)
    //        pos.y -= hit.tHit * dt;
    //    jumpSpeed = (pI->IsKeyPressed(VK_SPACE, false)) ? 9.8 : 0.0f;
    //    speed = (pI->IsKeyPressed(VK_SHIFT)) ? SPEED * 2.0f : SPEED;
    //}
    //else
    //{
    //    jumpSpeed -= 9.8f * dt;
    //}

    //const float dx = pI->GetXAxis();
    //const float dz = pI->GetZAxis();

    //Vector3 forward = m_pPlayer->GetForward();
    //Vector3 right = Vector3::Up.Cross(forward);

    //pos += (right * dx + forward * dz) * speed * dt;
    //pos.y += jumpSpeed * dt;
    //m_pPlayer->SetPosition(pos.x, pos.y, pos.z);
}
