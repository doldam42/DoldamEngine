#include "pch.h"

#include "Client.h"
#include "SceneRegistry.h"

#include "PortalScene.h"

REGISTER_SCENE(PortalScene)

void PortalScene::Load()
{
    IGameManager    *pGame = g_pClient->GetGameManager();
    IPhysicsManager *pPhysics = g_pClient->GetPhysics();
    IRenderer       *pRenderer = pGame->GetRenderer();

    // Create Material
    Material reflectiveMaterial = {};
    reflectiveMaterial.metallicFactor = 0.0f;
    reflectiveMaterial.reflectionFactor = 0.7f;
    wcscpy_s(reflectiveMaterial.name, L"ground");
    wcscpy_s(reflectiveMaterial.basePath, L"..\\..\\assets\\textures\\Tiles074\\");
    wcscpy_s(reflectiveMaterial.albedoTextureName, L"Tiles074_2K-JPG_Color.jpg");
    wcscpy_s(reflectiveMaterial.normalTextureName, L"Tiles074_2K-JPG_NormalDX.jpg");
    wcscpy_s(reflectiveMaterial.roughnessTextureName, L"Tiles074_2K-JPG_Roughness.jpg");
    IRenderMaterial *pGroundMaterial = pRenderer->CreateMaterialHandle(&reflectiveMaterial);

    IGameModel  *pGroundModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_BOX);
    IGameObject *pGround = pGame->CreateGameObject();
    pGround->SetModel(pGroundModel);
    pGround->SetPosition(0.0f, 0.0f, 0.0f);
    pGround->SetScale(30.0f, 0.2f, 30.0f);
    pGround->SetMaterials(&pGroundMaterial, 1);

    ICollider *pCollider = pPhysics->CreateBoxCollider(pGround, Vector3(30.0f, 0.2f, 30.0f));
    pGround->SetCollider(pCollider);

    IGameObject *pCeiling = pGame->CreateGameObject();
    pCeiling->SetModel(pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_BOX));
    pCeiling->SetPosition(0.0f, 60.0f, 0.0f);
    pCeiling->SetScale(30.0f, 0.2f, 30.0f);
    pCeiling->SetMaterials(&pGroundMaterial, 1);
    //pCeiling->SetCollider(pCollider = pPhysics->CreateBoxCollider(pCeiling, Vector3(30.0f, 0.2f, 30.0f)));

    // Walls
    float x, y, z;
    y = 30.0f;
    // +z, +x, -z, -x
    for (int i = 0; i < 4; i++)
    {
        float sign = (i < 2) ? 1.0f : -1.0f;
        if (i % 2 == 1)
        {
            x = sign * 30.f;
            z = 0.2f;
        }
        else
        {
            x = 0.2f;
            z = sign * 30.0f;
        }

        IGameObject *pWall = pGame->CreateGameObject();
        pWall->SetModel(pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_BOX));
        pWall->SetPosition(z, 30.0f, x);
        pWall->SetScale(x, y, z);
        pWall->SetMaterials(&pGroundMaterial, 1);
        pWall->SetCollider(pPhysics->CreateBoxCollider(pWall, Vector3(x, y, z)));
    }
}

void PortalScene::Update(float dt) {}

void PortalScene::UnLoad() {}
