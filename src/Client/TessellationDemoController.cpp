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
    Material groundMaterial = {};
    groundMaterial.metallicFactor = 0.0f;

    wcscpy_s(groundMaterial.name, L"ground");
    /*wcscpy_s(groundMaterial.basePath, L"..\\..\\assets\\textures\\Tiles074\\");
    wcscpy_s(groundMaterial.albedoTextureName, L"Tiles074_2K-JPG_Color.jpg");
    wcscpy_s(groundMaterial.normalTextureName, L"Tiles074_2K-JPG_NormalDX.jpg");
    wcscpy_s(groundMaterial.roughnessTextureName, L"Tiles074_2K-JPG_Roughness.jpg");
    wcscpy_s(groundMaterial.heightTextureName, L"Tiles074_2K-JPG_Displacement.jpg");*/
    wcscpy_s(groundMaterial.basePath, L"..\\..\\assets\\textures\\Bricks097\\");
    wcscpy_s(groundMaterial.albedoTextureName, L"Bricks097_2K-JPG_Color.jpg");
    wcscpy_s(groundMaterial.normalTextureName, L"Bricks097_2K-JPG_NormalDX.jpg");
    wcscpy_s(groundMaterial.aoTextureName, L"Bricks097_2K-JPG_AmbientOcclusion.jpg");
    wcscpy_s(groundMaterial.roughnessTextureName, L"Bricks097_2K-JPG_Roughness.jpg");
    wcscpy_s(groundMaterial.heightTextureName, L"Bricks097_2K-JPG_Displacement.jpg");

    pGame->CreateTerrain(&groundMaterial, 4, 4, 20);

    // Set Camera Position
    pGame->SetCameraPosition(-0.0f, 10.0f, -3.0f);


    return TRUE;
}

void TessellationDemoController::Update(float dt) {}

TessellationDemoController::~TessellationDemoController() { Cleanup(); }
