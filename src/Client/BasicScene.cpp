#include "pch.h"

#include "Client.h"

#include "BasicScene.h"

REGISTER_SCENE(BasicScene)

void BasicScene::Load() 
{
    IGameManager *pGame = g_pClient->GetGameManager();
    IPhysicsManager *pPhysics = g_pClient->GetPhysics();
    IRenderer       *pRenderer = pGame->GetRenderer();

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

    IGameModel  *pGroundModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_BOX);
    IGameObject *pGround = pGame->CreateGameObject();
    pGround->SetModel(pGroundModel);
    pGround->SetPosition(0.0f, 0.0f, 0.0f);
    pGround->SetScale(30.0f, 0.2f, 30.0f);
    pGround->SetMaterials(&pGroundMaterial, 1);

    ICollider  *pCollider = pPhysics->CreateBoxCollider(Vector3(30.0f, 0.2f, 30.0f));
    IRigidBody *pBody = pPhysics->CreateRigidBody(pCollider, Vector3::Zero, 0.0f, 0.5f, 0.5f, FALSE);
    pGround->SetCollider(pCollider);
    pGround->SetRigidBody(pBody);

    // Set CrossHair
    {
        UINT width = g_pClient->GetScreenWidth();
        UINT height = g_pClient->GetScreenHeight();
        m_crossHairPosX = (width / 2) - m_crossHairImageSize * m_crossHairScale * 0.5f;
        m_crossHairPosY = (height / 2) - m_crossHairImageSize * m_crossHairScale * 0.5f;

        IGameSprite *pSprite = pGame->CreateSpriteFromFile(L"../../assets/textures/", L"crosshair.dds",
                                                             m_crossHairImageSize, m_crossHairImageSize);
        pSprite->SetScale(0.25);
        pSprite->SetPosition(m_crossHairPosX, m_crossHairPosY);

        m_pCrossHairSprite = pSprite;
    }

    pGame->SetCameraPosition(0.0f, 2.0f, -2.0f);
}

void BasicScene::Update(float dt)
{
    if (g_pClient->IsWindowResized())
    {
        UINT width =  g_pClient->GetScreenWidth();
        UINT height = g_pClient->GetScreenHeight();
        m_crossHairPosX = (width / 2) - m_crossHairImageSize * m_crossHairScale * 0.5f;
        m_crossHairPosY = (height / 2) - m_crossHairImageSize * m_crossHairScale * 0.5f;
        m_pCrossHairSprite->SetPosition(m_crossHairPosX, m_crossHairPosY);
    }
}

void BasicScene::UnLoad() 
{ 
    IGameManager *pGame = g_pClient->GetGameManager();
    if (m_pCrossHairSprite)
    {
        pGame->DeleteSprite(m_pCrossHairSprite);
        m_pCrossHairSprite = nullptr;
    }
}
