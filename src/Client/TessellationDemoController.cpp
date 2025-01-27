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
    wcscpy_s(groundMaterial.basePath, L"..\\..\\assets\\textures\\terrain\\rocky\\");
    wcscpy_s(groundMaterial.albedoTextureName, L"diffuse.dds");
    wcscpy_s(groundMaterial.normalTextureName, L"normal.dds");
    wcscpy_s(groundMaterial.roughnessTextureName, L"roughness.jpg");
    wcscpy_s(groundMaterial.metallicTextureName, L"metallic.jpg");
    wcscpy_s(groundMaterial.heightTextureName, L"height.jpg");

    Vector3 scale(20.0f, 1.0f, 20.0f);
    pGame->CreateTerrain(&groundMaterial, &scale, 256, 256);

    // Set Camera Position
    pGame->SetCameraPosition(-0.0f, 2.0f, -3.0f);

    return TRUE;
}

void TessellationDemoController::Update(float dt) {}

TessellationDemoController::~TessellationDemoController() { Cleanup(); }
