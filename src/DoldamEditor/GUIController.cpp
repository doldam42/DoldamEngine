#include "pch.h"

#include "FileDialogUtils.h"
#include "FileManager.h"
#include "GUIView.h"
#include "GameEditor.h"

#include "GUIController.h"

void GUIController::CreateModel(const WCHAR *basePath, const WCHAR *filename)
{
    IGameManager   *pGame = g_pEditor->GetGameManager();
    IModelExporter *pExporter = g_pEditor->GetFBXModelExporter();

    WCHAR ext[20] = {L'\0'};
    WCHAR tmp[MAX_NAME] = {'\0'};
    wcscpy_s(tmp, filename);

    TryGetExtension(filename, ext);
    if (!wcscmp(ext, L".fbx"))
    {
        std::filesystem::path p(basePath);
        p /= filename;
        
        pExporter->Load(basePath, filename);
        pExporter->ExportModel();

        ChangeExtension(L".dom", tmp);
    }

    IGameModel *pModel = pGame->CreateModelFromFile(basePath, tmp);
    m_pGUIView->models.push_back(pModel);
}

void GUIController::CreateGameObject(const WCHAR *basePath, const WCHAR *filename)
{
    IGameManager   *pGame = g_pEditor->GetGameManager();
    IModelExporter *pExporter = g_pEditor->GetFBXModelExporter();

    WCHAR ext[20] = {L'\0'};
    WCHAR tmp[MAX_NAME] = {'\0'};
    wcscpy_s(tmp, filename);

    TryGetExtension(filename, ext);
    if (!wcscmp(ext, L".fbx"))
    {
        std::filesystem::path p(basePath);
        p /= filename;

        pExporter->Load(basePath, filename);
        pExporter->ExportModel();

        ChangeExtension(L".dom", tmp);
    }

    IGameModel *pModel = pGame->CreateModelFromFile(basePath, tmp);
    m_pGUIView->models.push_back(pModel);

    IGameObject *pObj = pGame->CreateGameObject();
    pObj->SetModel(pModel);
    pObj->SetScale(2.0f);
    m_pGUIView->objects.push_back(pObj);

    pObj->SetPosition(0.0f, 2.0f, 0.0f);
}

void GUIController::Cleanup()
{
    if (m_pGUIView)
    {
        delete m_pGUIView;
        m_pGUIView = nullptr;
    }
}

BOOL GUIController::Initialize(IRenderer *pRnd, FileNode *assetDir, const WCHAR *basePath, UINT width, UINT height)
{
    m_pGUIView = new GUIView(pRnd->GetRenderGUI(), pRnd->GetRenderTargetTexture(), assetDir, basePath, width, height);

    m_pRenderer = pRnd;
    return TRUE;
}

void GUIController::OnUpdateWindowSize(UINT width, UINT height)
{
    m_pGUIView->width = width;
    m_pGUIView->height = height;
}

BOOL GUIController::Start()
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

    // Set Camera Position
    pGame->SetCameraPosition(-0.0f, 2.0f, -5.0f);

    m_pTex = pGroundMaterial->GetTexture(TEXTURE_TYPE_ALBEDO);

    return TRUE;
}

void GUIController::Update(float dt)
{
    if (m_pGUIView->isItemSelected)
    {
        switch (m_pGUIView->currentRequestType)
        {
        case GUI_REQUEST_TYPE_CREATE_MODEL:
            CreateModel(m_pGUIView->basePath, m_pGUIView->filename);
            break;
        case GUI_REQUEST_TYPE_CREATE_GAMEOBJECT:
            CreateGameObject(m_pGUIView->basePath, m_pGUIView->filename);
            break;
        default:
            break;
        }

        m_pGUIView->isItemSelected = false;
        m_pGUIView->currentRequestType = GUI_REQUEST_TYPE_NONE;
    }
}

void GUIController::Render()
{
    m_pGUIView->pSceneViewTex = m_pRenderer->GetRenderTargetTexture();
    m_pGUIView->ShowSceneView();
    m_pGUIView->ShowHierarchy();
    m_pGUIView->ShowProject();
    m_pGUIView->ShowInspector();
}

Vector2 GUIController::GetViewportSizeRatio() { return m_pGUIView->SceneViewSize; }

GUIController::~GUIController() { Cleanup(); }
