#include "pch.h"

#include <filesystem>

#include "GameEditor.h"

#include "SceneViewerController.h"

void SceneViewerController::Cleanup()
{
    IGameManager *pGame = g_pEditor->GetGameManager();
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

BOOL SceneViewerController::Start()
{
    IGameManager *pGame = g_pEditor->GetGameManager();
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

    Material wallMaterial = {};
    wcscpy_s(wallMaterial.name, L"wall");
    wcscpy_s(wallMaterial.basePath, L"..\\..\\assets\\textures\\Bricks097\\");
    wcscpy_s(wallMaterial.albedoTextureName, L"Bricks097_2K-JPG_Color.DDS");
    wcscpy_s(wallMaterial.normalTextureName, L"Bricks097_2K-JPG_NormalDX.jpg");
    wcscpy_s(wallMaterial.aoTextureName, L"Bricks097_2K-JPG_AmbientOcclusion.jpg");
    wcscpy_s(wallMaterial.roughnessTextureName, L"Bricks097_2K-JPG_Roughness.jpg");
    wcscpy_s(wallMaterial.heightTextureName, L"Bricks097_2K-JPG_Displacement.jpg");
    wallMaterial.metallicFactor = 0.0f;

    Material ChristmasTreeOrnamentMaterial = {};
    wcscpy_s(ChristmasTreeOrnamentMaterial.name, L"ChristmasTreeOrnament");
    wcscpy_s(ChristmasTreeOrnamentMaterial.basePath, L"..\\..\\assets\\textures\\ChristmasTreeOrnament014\\");
    wcscpy_s(ChristmasTreeOrnamentMaterial.albedoTextureName, L"ChristmasTreeOrnament014_2K-JPG_Color.jpg");
    wcscpy_s(ChristmasTreeOrnamentMaterial.normalTextureName, L"ChristmasTreeOrnament014_2K-JPG_NormalDX.jpg");
    wcscpy_s(ChristmasTreeOrnamentMaterial.roughnessTextureName, L"ChristmasTreeOrnament014_2K-JPG_Roughness.jpg");
    wcscpy_s(ChristmasTreeOrnamentMaterial.metallicTextureName, L"ChristmasTreeOrnament014_2K-JPG_Metalness.jpg");
    ChristmasTreeOrnamentMaterial.reflectionFactor = 0.2f;

    IRenderMaterial *pGroundMaterial = pRenderer->CreateMaterialHandle(&reflectiveMaterial);
    IRenderMaterial *pWallMaterial = pRenderer->CreateMaterialHandle(&wallMaterial);
    IRenderMaterial *pChristmasTreeOrnamentMaterial = pRenderer->CreateMaterialHandle(&ChristmasTreeOrnamentMaterial);

    // Create Model & GameObject
    IGameModel *pBoxModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_BOX);
    IGameModel *pSquareModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_SQUARE);
    IGameModel *pSphereModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_SPHERE);

    IGameObject *pGround = pGame->CreateGameObject();
    pGround->SetModel(pSquareModel);
    pGround->SetPosition(0.0f, -1e-3, 0.f);
    pGround->SetRotationX(-XM_PIDIV2);
    pGround->SetScale(20.0f);
    pGround->SetMaterials(&pGroundMaterial, 1);

    IGameObject *pBox = pGame->CreateGameObject();
    pBox->SetModel(pBoxModel);
    pBox->SetScale(1.5f, 1.5f, 1.5f);
    pBox->SetRotationZ(XM_PIDIV2);
    pBox->SetPosition(3.0f, 1.5f, 0.0f);
    pBox->SetMaterials(&pWallMaterial, 1);

    IGameObject *pSphere = pGame->CreateGameObject();
    pSphere->SetModel(pSphereModel);
    pSphere->SetScale(2.0f);
    pSphere->SetPosition(-5.0f, 2.0f, 0.0f);
    pSphere->SetMaterials(&pChristmasTreeOrnamentMaterial, 1);

    m_pBox = pBox;
    m_pGround = pGround;
    /*IModelExporter       *pExporter = g_pClient->GetAssimpExporter();
    std::filesystem::path p = L"..\\..\\assets\\sponza\\NewSponza_Main_glTF_003.dom";
    if (!std::filesystem::exists(p))
    {
        pExporter->Load(L"../../assets/sponza/", L"NewSponza_Main_glTF_003.gltf");
        pExporter->ExportModel();
    }

    IGameModel  *pSponzaModel = pGame->CreateModelFromFile(L"../../assets/sponza/", L"NewSponza_Main_glTF_003.dom");
    IGameObject *pSponza = pGame->CreateGameObject();
    pSponza->SetModel(pSponzaModel);
    pSponza->SetScale(20.f);*/

    // Set Camera Position
    pGame->SetCameraPosition(-0.0f, 2.0f, -5.0f);

    return TRUE;
}

void SceneViewerController::Update(float dt) {}

SceneViewerController::~SceneViewerController() { Cleanup(); }

void SceneViewerController::Render() {}
