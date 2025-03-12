#include "pch.h"

#include <filesystem>

#include "AudioManager.h"
#include "InputManager.h"
#include "VideoManager.h"

#include "ControllerRegistry.h"

#include "BadAppleController.h"
#include "CameraController.h"
#include "CollisionDemoController.h"
#include "PhysicsDemoController.h"
#include "RaytracingDemoController.h"
#include "TessellationDemoController.h"
#include "TimeController.h"

#include "Client.h"

namespace fs = std::filesystem;

Client *g_pClient = nullptr;

BOOL Client::LoadModules(HWND hWnd)
{
    BOOL                 result = FALSE;
    CREATE_INSTANCE_FUNC pCreateFunc;

    const WCHAR *rendererFileName = nullptr;
    const WCHAR *engineFileName = nullptr;
    const WCHAR *exporterFileName = nullptr;
#ifdef _DEBUG
    // rendererFileName = L"../../DLL/RendererD3D12_x64_debug.dll";
    rendererFileName = L"../../DLL/RendererRaytracing_x64_Debug.dll";
    engineFileName = L"../../DLL/EngineModule_x64_Debug.dll";
    exporterFileName = L"../../DLL/ModelExporter_x64_Debug.dll";
#else
    rendererFileName = L"../../DLL/RendererRaytracing_x64_Release.dll";
    engineFileName = L"../../DLL/EngineModule_x64_Release.dll";
    exporterFileName = L"../../DLL/ModelExporter_x64_Release.dll";
#endif

    WCHAR wchErrTxt[128] = {};
    DWORD dwErrCode = 0;

    m_hRendererDLL = LoadLibrary(rendererFileName);
    if (!m_hRendererDLL)
    {
        dwErrCode = GetLastError();
        swprintf_s(wchErrTxt, L"Fail to LoadLibrary(%s) - Error Code: %u", rendererFileName, dwErrCode);
        MessageBox(hWnd, wchErrTxt, L"Error", MB_OK);
        __debugbreak();
    }
    pCreateFunc = (CREATE_INSTANCE_FUNC)GetProcAddress(m_hRendererDLL, "DllCreateInstance");
    pCreateFunc(&m_pRenderer);

    m_hEngineDLL = LoadLibrary(engineFileName);
    if (!m_hEngineDLL)
    {
        dwErrCode = GetLastError();
        swprintf_s(wchErrTxt, L"Fail to LoadLibrary(%s) - Error Code: %u", engineFileName, dwErrCode);
        MessageBox(hWnd, wchErrTxt, L"Error", MB_OK);
        __debugbreak();
    }
    pCreateFunc = (CREATE_INSTANCE_FUNC)GetProcAddress(m_hEngineDLL, "DllCreateInstance");
    pCreateFunc(&m_pGame);

    m_hModelExporterDLL = LoadLibrary(exporterFileName);
    if (!m_hModelExporterDLL)
    {
        dwErrCode = GetLastError();
        swprintf_s(wchErrTxt, L"Fail to LoadLibrary(%s) - Error Code: %u", exporterFileName, dwErrCode);
        MessageBox(hWnd, wchErrTxt, L"Error", MB_OK);
        __debugbreak();
    }
    pCreateFunc = (CREATE_INSTANCE_FUNC)GetProcAddress(m_hModelExporterDLL, "DllCreateAssimpLoader");
    pCreateFunc(&m_pAssimpExporter);

    pCreateFunc = (CREATE_INSTANCE_FUNC)GetProcAddress(m_hModelExporterDLL, "DllCreateFbxLoader");
    pCreateFunc(&m_pFbxExporter);

    result = m_pRenderer->Initialize(hWnd, TRUE, FALSE);
    result = m_pGame->Initialize(hWnd, m_pRenderer);
    result = m_pAssimpExporter->Initialize(m_pGame);
    result = m_pFbxExporter->Initialize(m_pGame);

    if (!result)
        __debugbreak();

    return result;
}

void Client::CleanupModules()
{
    // Cleanup Modules
    if (m_pFbxExporter)
    {
        m_pFbxExporter->Release();
        m_pFbxExporter = nullptr;
    }
    if (m_pAssimpExporter)
    {
        m_pAssimpExporter->Release();
        m_pAssimpExporter = nullptr;
    }
    if (m_pRenderer)
    {
        m_pRenderer->Release();
        m_pRenderer = nullptr;
    }
    if (m_pGame)
    {
        m_pGame->Release();
        m_pGame = nullptr;
    }
    // Cleanup DLLs
    if (m_hModelExporterDLL)
    {
        FreeLibrary(m_hModelExporterDLL);
        m_hModelExporterDLL = nullptr;
    }
    if (m_hEngineDLL)
    {
        FreeLibrary(m_hEngineDLL);
        m_hEngineDLL = nullptr;
    }
    if (m_hRendererDLL)
    {
        FreeLibrary(m_hRendererDLL);
        m_hRendererDLL = nullptr;
    }
}

void Client::ProcessInput()
{
    if (m_pInputManager->IsKeyPressed(VK_ESCAPE))
    {
        DestroyWindow(m_hWnd);
    }
    if (m_pInputManager->IsKeyPressed('P', false))
    {
        m_isPaused = !m_isPaused;
    }

    m_pInputManager->ProcessInput();
}

void Client::CleanupControllers()
{
    if (m_pCameraController)
    {
        delete m_pCameraController;
        m_pCameraController = nullptr;
    }

    if (m_pAudioManager)
    {
        delete m_pAudioManager;
        m_pAudioManager = nullptr;
    }
}

void Client::Cleanup()
{
    if (m_pInputManager)
    {
        delete m_pInputManager;
        m_pInputManager = nullptr;
    }

    CleanupControllers();

    CleanupModules();
}

BOOL Client::Initialize(HWND hWnd)
{
    BOOL result = FALSE;

    result = LoadModules(hWnd);

    RECT rect;
    GetClientRect(hWnd, &rect);
    DWORD width = rect.right - rect.left;
    DWORD height = rect.bottom - rect.top;

    m_width = width;
    m_height = height;

    m_pInputManager = new InputManager;
    m_pInputManager->Initialize(width, height);

    m_pAudioManager = new AudioManager;
    m_pCameraController = new CameraController;
    m_pGame->Register(m_pAudioManager);
    m_pGame->Register(m_pCameraController);

    ControllerRegistry::GetInstance().RegisterAll(m_pGame);

    Start();

    m_hWnd = hWnd;

    m_prevUpdateTick = m_prevFrameCheckTick = GetTickCount64();

    return result;
}

void Client::LoadResources() {}

void Client::LoadScene()
{
    // Create Material
    Material reflectiveMaterial = {};
    reflectiveMaterial.metallicFactor = 0.0f;
    reflectiveMaterial.reflectionFactor = 0.9f;
    wcscpy_s(reflectiveMaterial.name, L"ground");
    wcscpy_s(reflectiveMaterial.basePath, L"..\\..\\assets\\textures\\Tiles074\\");
    wcscpy_s(reflectiveMaterial.albedoTextureName, L"Tiles074_2K-JPG_Color.jpg");
    wcscpy_s(reflectiveMaterial.normalTextureName, L"Tiles074_2K-JPG_NormalDX.jpg");
    wcscpy_s(reflectiveMaterial.roughnessTextureName, L"Tiles074_2K-JPG_Roughness.jpg");
    IRenderMaterial *pGroundMaterial = m_pRenderer->CreateMaterialHandle(&reflectiveMaterial);

    IGameModel *pGroundModel = m_pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_BOX);
    IGameObject *pGround = m_pGame->CreateGameObject(TRUE);
    pGround->SetModel(pGroundModel);
    pGround->SetPosition(0.0f, 0.0f, 0.0f);
    pGround->SetScale(30.0f, 0.2f, 30.0f);
    pGround->SetMaterials(&pGroundMaterial, 1);
    pGround->InitBoxCollider(Vector3::Zero, Vector3(30.0f, 0.2f, 30.0f));
    pGround->InitRigidBody(0.0f, 0.5f, 0.5f, FALSE);

    // pGround->InitBoxCollider(Vector3::Zero, Vector3(25.0f, 0.2f, 25.0f));

    // pGround->InitRigidBody(0.0f, 0.5f, 0.0f, FALSE, FALSE);

    //IGameModel  *pSphereModel = m_pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_SPHERE);
    //IGameObject *pSphere = m_pGame->CreateGameObject();
    //pSphere->SetModel(pSphereModel);
    //pSphere->SetPosition(0.0f, 10.0f, 0.0f);
    //// pGround->SetMaterials(&pGroundMaterial, 1);
    //pSphere->InitSphereCollider(Vector3::Zero, 1.0f);
    //pSphere->InitRigidBody(1.0f, 0.5f, 0.5f, TRUE, FALSE);

    // Set CrossHair
    {
        UINT width = g_pClient->GetScreenWidth();
        UINT height = g_pClient->GetScreenHeight();

        int posX = (width / 2) - 32;
        int posY = (height / 2) - 32;

        IGameSprite *pSprite = m_pGame->CreateSpriteFromFile(L"../../assets/textures/", L"crosshair.dds", 256, 256);
        pSprite->SetScale(0.25);
        pSprite->SetPosition(posX, posY);
    }
    
    m_pGame->SetCameraPosition(0.0f, 2.0f, -2.0f);
}

void Client::Process()
{
    ProcessInput();

    ULONGLONG curTick = GetTickCount64();
    float     dt = static_cast<float>(curTick - m_prevUpdateTick) / 1000.f;

    // FPS가 너무 낮은 경우 30 FPS로 고정
    dt = (dt > 0.1f) ? 0.03f : dt;
    if (!m_isPaused && dt > 0.025f) // // Update Scene with 40FPS
    {
        m_prevUpdateTick = curTick;
        dt *= m_timeSpeed;

        m_pGame->Update(dt);

        m_pInputManager->Update();
    }

    m_pGame->Render();

    // 성능 측정
    m_frameCount++;
    if (curTick - m_prevFrameCheckTick > 1000)
    {
        m_prevFrameCheckTick = curTick;
        m_FPS = m_frameCount;
        m_frameCount = 0;
    }
}

BOOL Client::Start()
{
    m_pGame->Start();

    LoadResources();

    LoadScene();

    m_pGame->BuildScene();

    return TRUE;
}

void Client::OnKeyDown(UINT nChar, UINT uiScanCode) { m_pInputManager->OnKeyDown(nChar, uiScanCode); }

void Client::OnKeyUp(UINT nChar, UINT uiScanCode) { m_pInputManager->OnKeyUp(nChar, uiScanCode); }

void Client::OnMouseMove(int mouseX, int mouseY) { m_pInputManager->OnMouseMove(mouseX, mouseY); }

void Client::OnMouseWheel(float deltaWheel) { m_pInputManager->OnMouseWheel(deltaWheel); }

void Client::OnMouseLButtonDown() { m_pInputManager->OnMouseLButtonDown(); }

void Client::OnMouseLButtonUp() { m_pInputManager->OnMouseLButtonUp(); }

void Client::OnMouseRButtonDown() { m_pInputManager->OnMouseRButtonDown(); }

void Client::OnMouseRButtonUp() { m_pInputManager->OnMouseRButtonUp(); }

BOOL Client::OnUpdateWindowSize(UINT width, UINT height)
{
    m_width = width;
    m_height = height;

    m_pInputManager->SetWindowSize(width, height);

    return m_pGame->OnUpdateWindowSize(width, height);
}

Client::~Client() { Cleanup(); }
