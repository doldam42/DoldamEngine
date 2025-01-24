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
    wcscpy_s(groundMaterial.basePath, L"..\\..\\assets\\textures\\heightMap\\");
    wcscpy_s(groundMaterial.albedoTextureName, L"color.bmp");
    //wcscpy_s(groundMaterial.normalTextureName, L"Bricks097_2K-JPG_NormalDX.jpg");
    //wcscpy_s(groundMaterial.aoTextureName, L"Bricks097_2K-JPG_AmbientOcclusion.jpg");
    //wcscpy_s(groundMaterial.roughnessTextureName, L"Bricks097_2K-JPG_Roughness.jpg");
    wcscpy_s(groundMaterial.heightTextureName, L"height.bmp");

    pGame->CreateTerrain(&groundMaterial, 16, 16, 100.0f);

    // Set Camera Position
    pGame->SetCameraPosition(-0.0f, 10.0f, -3.0f);


    return TRUE;
}

void TessellationDemoController::Update(float dt) {}

TessellationDemoController::~TessellationDemoController() { Cleanup(); }
