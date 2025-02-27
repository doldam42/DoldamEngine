#include "pch.h"

#include <filesystem>

#include "AudioManager.h"
#include "VideoManager.h"
#include "InputManager.h"

#include "CameraController.h"
#include "BadAppleController.h"
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
    //rendererFileName = L"../../DLL/RendererD3D12_x64_debug.dll";
    rendererFileName = L"../../DLL/RendererRaytracing_x64_debug.dll";
    engineFileName = L"../../DLL/EngineModule_x64_debug.dll";
    exporterFileName = L"../../DLL/ModelExporter_x64_debug.dll";
#else
    rendererFileName = L"../../DLL/RendererD3D12_x64_release.dll";
    engineFileName = L"../../DLL/EngineModule_x64_release.dll";
    exporterFileName = L"../../DLL/ModelExporter_x64_release.dll";
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

void Client::ProcessInput() 
{
    if (m_pInputManager->IsKeyPressed(VK_ESCAPE))
    {
        DestroyWindow(m_hWnd);
    }
}

void Client::CleanupControllers()
{
    if (m_pCameraController)
    {
        delete m_pCameraController;
        m_pCameraController = nullptr;
    }
    if (m_pDemoController)
    {
        delete m_pDemoController;
        m_pDemoController = nullptr;
    }
    if (m_pTimeController)
    {
        delete m_pTimeController;
        m_pTimeController = nullptr;
    }
    if (m_pRaytracingDemoController)
    {
        delete m_pRaytracingDemoController;
        m_pRaytracingDemoController = nullptr;
    }
    if (m_pTessellationDemoController)
    {
        delete m_pTessellationDemoController;
        m_pTessellationDemoController = nullptr;
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
    if (m_pAudio)
    {
        delete m_pAudio;
        m_pAudio = nullptr;
    }
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

BOOL Client::Initialize(HWND hWnd)
{
    BOOL result = FALSE;

    result = LoadModules(hWnd);

    RECT rect;
    GetClientRect(hWnd, &rect);
    DWORD width = rect.right - rect.left;
    DWORD height = rect.bottom - rect.top;

    m_pInputManager = new InputManager;
    m_pInputManager->Initialize(width, height);

    m_pAudio = new AudioManager;
    m_pAudio->Initialize();

    m_pCameraController = new CameraController;
    m_pCameraController->Initialize(this);
    m_pGame->Register(m_pCameraController);

    // Register Controllers Before Start Game Manager.
    m_pTimeController = new TimeController;
    m_pRaytracingDemoController = new RaytracingDemoController;
     //m_pTessellationDemoController = new TessellationDemoController();

    m_pGame->Register(m_pAudio);
    m_pGame->Register(m_pTimeController);
    m_pGame->Register(m_pRaytracingDemoController);
     //m_pGame->Register(m_pTessellationDemoController);

    Start();

    m_hWnd = hWnd;

    return result;
}

void Client::LoadResources() {}

void Client::LoadScene() {}

void Client::Process()
{
    ProcessInput();

    m_pGame->Update();

    m_pGame->Render();
}

BOOL Client::Start()
{
    LoadResources();

    m_pGame->Start();

    LoadScene();
    m_pGame->BuildScene();

    return TRUE;
}

void Client::OnKeyDown(UINT nChar, UINT uiScanCode) { m_pInputManager->OnKeyDown(nChar, uiScanCode); }

void Client::OnKeyUp(UINT nChar, UINT uiScanCode) { m_pInputManager->OnKeyUp(nChar, uiScanCode); }

void Client::OnMouseMove(int mouseX, int mouseY) { m_pInputManager->OnMouseMove(mouseX, mouseY); }

void Client::OnMouseWheel(float deltaWheel) { m_pInputManager->OnMouseWheel(deltaWheel); }

BOOL Client::OnUpdateWindowSize(UINT width, UINT height)
{
    m_pInputManager->SetWindowSize(width, height);
    return m_pGame->OnUpdateWindowSize(width, height);
}

Client::~Client() { Cleanup(); }
