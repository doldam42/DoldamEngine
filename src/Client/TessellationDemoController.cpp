#include "pch.h"
#include "Client.h"
#include "TessellationDemoController.h"

void TessellationDemoController::Cleanup() 
{ 
    IGameManager *pGame = g_pClient->GetGameManager();
    if (m_pGround)
    {
        pGame->DeleteGameObject(m_pGround);
        m_pGround = nullptr;
    }
}

BOOL TessellationDemoController::Start() 
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

    IRenderMaterial *pGroundMaterial = pRenderer->CreateMaterialHandle(&reflectiveMaterial);
    IGameModel      *pSquareModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_SQUARE);

    IGameObject *pGround = pGame->CreateGameObject();
    pGround->SetModel(pSquareModel);
    pGround->SetPosition(0.0f, -1e-3, 0.f);
    pGround->SetRotationX(-XM_PIDIV2);
    pGround->SetScale(20.0f);
    pGround->SetMaterials(&pGroundMaterial, 1);

    // Set Camera Position
    pGame->SetCameraPosition(-0.0f, 2.0f, -5.0f);

    return TRUE;
}

void TessellationDemoController::Update(float dt) {}

TessellationDemoController::~TessellationDemoController() { Cleanup(); }
