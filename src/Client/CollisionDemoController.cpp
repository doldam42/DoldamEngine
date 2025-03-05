#include "pch.h"

#include "CameraController.h"
#include "Client.h"
#include "InputManager.h"

#include "CollisionDemoController.h"

BOOL CollisionDemoController::Start()
{
    CameraController *pCam = g_pClient->GetCameraController();
    IGameManager     *pGame = g_pClient->GetGameManager();
    IRenderer        *pRenderer = pGame->GetRenderer();

    // Create Material
    Material reflectiveMaterial = {};
    reflectiveMaterial.metallicFactor = 0.0f;
    reflectiveMaterial.reflectionFactor = 0.9f;
    wcscpy_s(reflectiveMaterial.name, L"ground");
    wcscpy_s(reflectiveMaterial.basePath, L"..\\..\\assets\\textures\\Tiles074\\");
    wcscpy_s(reflectiveMaterial.albedoTextureName, L"Tiles074_2K-JPG_Color.jpg");
    wcscpy_s(reflectiveMaterial.normalTextureName, L"Tiles074_2K-JPG_NormalDX.jpg");
    wcscpy_s(reflectiveMaterial.roughnessTextureName, L"Tiles074_2K-JPG_Roughness.jpg");
    IRenderMaterial *pGroundMaterial = pRenderer->CreateMaterialHandle(&reflectiveMaterial);

    IGameModel  *pGroundModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_SQUARE);
    IGameObject *pGround = pGame->CreateGameObject();
    pGround->SetModel(pGroundModel);
    pGround->SetPosition(0.0f, 0.0f, 0.0f);
    pGround->SetRotationX(-XM_PIDIV2);
    pGround->SetScale(50.0f);
    pGround->SetMaterials(&pGroundMaterial, 1);
    //pGround->InitBoxCollider(Vector3::Zero, Vector3(25.0f, 0.01f, 25.0f));
    // pGround->InitRigidBody(0.0f, 1.0f, 0.5f);

    m_pPlayer = pGame->CreateGameObject();
    m_pPlayer->SetPosition(0.0f, 2.f, 0.0f);
    pCam->SetFollowTarget(m_pPlayer);

    UINT width = g_pClient->GetScreenWidth();
    UINT height = g_pClient->GetScreenHeight();

    int posX = (width / 2) - 32;
    int posY = (height / 2) - 32;

    IGameSprite *pSprite = pGame->CreateSpriteFromFile(L"../../assets/textures/", L"crosshair.dds", 256, 256);
    pSprite->SetScale(0.25);
    pSprite->SetPosition(posX, posY);

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

    Vector3 dir = m_pPlayer->GetForward();
    Vector3 right = Vector3::Up.Cross(dir);
    
    Vector3 deltaPos = (dir * dz + right * dx) * speed;
    deltaPos.y += jumpSpeed;
    deltaPos *= dt;
    m_pPlayer->AddPosition(deltaPos);
}

void CollisionDemoController::Render() {}
