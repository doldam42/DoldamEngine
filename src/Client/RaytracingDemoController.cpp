#include "pch.h"

#include "Client.h"
#include <filesystem>

#include "RaytracingDemoController.h"

void RaytracingDemoController::Cleanup()
{
    IGameManager *pGame = g_pClient->GetGameManager();
    if (m_pBox)
    {
        pGame->DeleteGameObject(m_pBox);
        m_pBox = nullptr;
    }

    if (m_pGround)
    {
        pGame->DeleteGameObject(m_pGround);
        m_pGround = nullptr;
    }
}

BOOL RaytracingDemoController::Start()
{
    IGameManager *pGame = g_pClient->GetGameManager();
    IRenderer    *pRenderer = pGame->GetRenderer();

    // Create Material
    Material reflectiveMaterial = {};
    reflectiveMaterial.roughnessFactor = 0.1f;
    reflectiveMaterial.metallicFactor = 0.0f;
    reflectiveMaterial.reflectionFactor = 0.9f;
    wcscpy_s(reflectiveMaterial.name, L"ground");
    wcscpy_s(reflectiveMaterial.basePath, L"..\\..\\assets\\textures\\Tiles074\\");
    wcscpy_s(reflectiveMaterial.albedoTextureName, L"Tiles074_2K-JPG_Color.DDS");
    wcscpy_s(reflectiveMaterial.normalTextureName, L"Tiles074_2K-JPG_NormalDX.jpg");
    wcscpy_s(reflectiveMaterial.roughnessTextureName, L"Tiles074_2K-JPG_Roughness.jpg");
    //wcscpy_s(reflectiveMaterial.metallicTextureName, L"Tiles074_1K-JPG_Roughness.jpg");

    IRenderMaterial *pGroundMaterial = pRenderer->CreateMaterialHandle(&reflectiveMaterial);

    Material wallMaterial = {};
    wallMaterial.roughnessFactor = 1.0f;
    wallMaterial.metallicFactor = 0.0f;
    wcscpy_s(wallMaterial.name, L"wall");
    wcscpy_s(wallMaterial.basePath, L"..\\..\\assets\\textures\\Bricks097\\");
    wcscpy_s(wallMaterial.albedoTextureName, L"Bricks097_2K-JPG_Color.DDS");
    wcscpy_s(wallMaterial.normalTextureName, L"Bricks097_2K-JPG_NormalDX.jpg");
    wcscpy_s(wallMaterial.roughnessTextureName, L"Bricks097_2K-JPG_Roughness.jpg");

    IRenderMaterial *pWallMaterial = pRenderer->CreateMaterialHandle(&wallMaterial);

    // Create Model & GameObject
    IGameModel  *pBoxModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_BOX);
    IGameObject *pGround = pGame->CreateGameObject();
    pGround->SetModel(pBoxModel);
    pGround->SetPosition(0.0f, -21.f, 0.f);
    pGround->SetScale(20.0f);
    pGround->SetMaterials(&pGroundMaterial, 1);

    IGameObject *pBox = pGame->CreateGameObject();
    pBoxModel->AddRef();
    pBox->SetModel(pBoxModel);
    pBox->SetScale(1.0f, 2.0f, 2.0f);
    pBox->SetPosition(-1.0f, 1.0f, 0.0f);
    pBox->SetMaterials(&pWallMaterial, 1);
    
    IModelExporter *pExporter = g_pClient->GetFBXModelExporter();

    std::filesystem::path p = L"..\\..\\assets\\characters\\gura\\gura.dom";
    if (!std::filesystem::exists(p))
    {
        pExporter->Load(L"../../assets/characters/gura/", L"gura.fbx");
        pExporter->ExportModel();
    }
    p = L"..\\..\\assets\\characters\\gura\\Smolgura_seafoamboy_anims.dca";
    if (!std::filesystem::exists(p))
    {
        pExporter->LoadAnimation(L"Smolgura_seafoamboy_anims.fbx");
        pExporter->ExportAnimation();
    }

    IGameModel     *pGuraModel = pGame->CreateModelFromFile(L"../../assets/characters/gura/", L"gura.dom");
    IGameAnimation *pAnim = pGame->CreateAnimationFromFile(L"../../assets/characters/gura/", L"Smolgura_seafoamboy_anims.dca");

    IGameCharacter *pGura = pGame->CreateCharacter();
    pGura->SetModel(pGuraModel);
    pGura->InsertAnimation(pAnim);

    pGura->SetPosition(2.0f, 1.0f, 1.0f);
    Quaternion q = Quaternion::CreateFromYawPitchRoll(Vector3(-XM_PIDIV2, XM_PI, 0.0f));
    pGura->SetRotation(&q);

    // Set Camera Position
    pGame->SetCameraPosition(-2.0f, 6.0f, -8.0f);

    m_pBox = pBox;
    m_pGround = pGround;

    return TRUE;
}

void RaytracingDemoController::Update(float dt) {}

RaytracingDemoController::~RaytracingDemoController() { Cleanup(); }
