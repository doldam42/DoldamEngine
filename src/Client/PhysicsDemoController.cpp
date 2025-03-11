#include "pch.h"
#include "Client.h"
#include "InputManager.h"
#include "PhysicsDemoController.h"

BOOL PhysicsDemoController::Start() 
{
    IGameManager *pGame = g_pClient->GetGameManager();
    IRenderer    *pRenderer = pGame->GetRenderer();

    // Create Material
    Material reflectiveMaterial = {};
    reflectiveMaterial.metallicFactor = 0.0f;
    reflectiveMaterial.reflectionFactor = 0.9f;
    wcscpy_s(reflectiveMaterial.name, L"ground");
    wcscpy_s(reflectiveMaterial.basePath, L"..\\..\\assets\\textures\\Tiles074\\");
    wcscpy_s(reflectiveMaterial.albedoTextureName, L"Tiles074_2K-JPG_Color.jpg");
    wcscpy_s(reflectiveMaterial.normalTextureName, L"Tiles074_2K-JPG_NormalDX.jpg");
    wcscpy_s(reflectiveMaterial.roughnessTextureName, L"Tiles074_2K-JPG_Roughness.jpg");

    Material ChristmasTreeOrnamentMaterial = {};
    ChristmasTreeOrnamentMaterial.reflectionFactor = 0.2f;
    wcscpy_s(ChristmasTreeOrnamentMaterial.name, L"ChristmasTreeOrnament");
    wcscpy_s(ChristmasTreeOrnamentMaterial.basePath, L"..\\..\\assets\\textures\\ChristmasTreeOrnament014\\");
    wcscpy_s(ChristmasTreeOrnamentMaterial.albedoTextureName, L"ChristmasTreeOrnament014_2K-JPG_Color.jpg");
    wcscpy_s(ChristmasTreeOrnamentMaterial.normalTextureName, L"ChristmasTreeOrnament014_2K-JPG_NormalDX.jpg");
    wcscpy_s(ChristmasTreeOrnamentMaterial.roughnessTextureName, L"ChristmasTreeOrnament014_2K-JPG_Roughness.jpg");
    wcscpy_s(ChristmasTreeOrnamentMaterial.metallicTextureName, L"ChristmasTreeOrnament014_2K-JPG_Metalness.jpg");
    
    IRenderMaterial *pGroundMaterial = pRenderer->CreateMaterialHandle(&reflectiveMaterial);
    IRenderMaterial *pSphereMaterial = pRenderer->CreateMaterialHandle(&ChristmasTreeOrnamentMaterial);

    // Create Model & GameObject
    IGameModel *pGroundModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_SPHERE);
    IGameObject *pGround = pGame->CreateGameObject(TRUE);
    pGround->SetModel(pGroundModel);
    pGround->SetPosition(0.0f, -50.5f, 0.0f);
    pGround->SetScale(50.0f);
    pGround->SetMaterials(&pGroundMaterial, 1);
    pGround->InitSphereCollider(Vector3::Zero, 50.0f);
    pGround->InitRigidBody(0.0f, 1.0f, 0.5f);

    for (int i = 0; i < 3; i++)
    {
        float        dx = -0.5f;
        float height = 5.0f + 3.0f * i;
        IGameModel  *pSphereModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_SPHERE);
        IGameObject *pSphere = pGame->CreateGameObject(FALSE);
        pSphere->SetModel(pSphereModel);
        pSphere->SetPosition(dx, height, 0.0f);
        pSphere->SetScale(2.0f);
        pSphere->SetMaterials(&pSphereMaterial, 1);
        pSphere->InitSphereCollider(Vector3::Zero, 2.0f);
        pSphere->InitRigidBody(1.0f, 1.0f, 0.5f);
        dx += 0.5f;
    }
    pSphereMaterial->AddRef();
    pSphereMaterial->AddRef();

    UINT width = g_pClient->GetScreenWidth();
    UINT height = g_pClient->GetScreenHeight();

    int posX = (width / 2) - 32;
    int posY = (height / 2) - 32;

    IGameSprite *pSprite = pGame->CreateSpriteFromFile(L"../../assets/textures/", L"crosshair.png", 256, 256);
    pSprite->SetScale(0.25);
    pSprite->SetPosition(posX, posY);

    // Set Camera Position
    pGame->SetCameraPosition(-0.0f, 2.0f, -5.0f);

    return TRUE;
}

void PhysicsDemoController::Update(float dt)
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
}
