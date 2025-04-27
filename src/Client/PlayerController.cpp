#include "pch.h"

#include "CameraController.h"
#include "Client.h"
#include "ControllerRegistry.h"
#include "InputManager.h"

#include "PlayerController.h"

REGISTER_CONTROLLER(PlayerController)

void PlayerController::Cleanup()
{
    if (m_pController)
    {
        delete m_pController;
        m_pController = nullptr;
    }
}

BOOL PlayerController::Start()
{
    IGameManager     *pGame = g_pClient->GetGameManager();
    CameraController *pCam = g_pClient->GetCameraController();

    const Vector3 pos(1.0f, 5.0f, -5.0f);

    m_pController = new CharacterController;
    m_pController->Initialize(pos, 2.0f, 0.5f);
    
    // pCam->SetFollowTarget(m_pController->GetGameObject(), Vector3(0.0f, 2.0f, -3.0f));
    pCam->SetFollowTarget(m_pController->GetGameObject(), Vector3(0.0f, 2.0f, 0.0f));

    return TRUE;
}

void PlayerController::Update(float dt)
{
    InputManager *pI = g_pClient->GetInputManager();
    IGameManager *pGame = g_pClient->GetGameManager();

    /*else if (pI->IsKeyPressed(VK_RBUTTON, false))
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
   }*/

    BOOL isGround = m_pController->OnGround();

    if (isGround && pI->IsKeyPressed(VK_SPACE, false))
    {
        m_pController->Jump(JUMP_SPEED);
    }
    else
    {
        const float speed = (pI->IsKeyPressed(VK_SHIFT)) ? SPEED * 2.0f : SPEED;
        const float dx = pI->GetXAxis();
        const float dz = pI->GetZAxis();

        Vector3 forward = m_pController->GetGameObject()->GetForward();
        Vector3 right = Vector3::Up.Cross(forward);

        m_pController->Move((right * dx + forward * dz) * speed);
    }
    m_pController->Update(dt);
}

PlayerController::~PlayerController() { Cleanup(); }
