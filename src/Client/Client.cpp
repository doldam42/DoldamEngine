#include "pch.h"

#include <filesystem>

#include "AudioManager.h"
#include "VideoManager.h"
#include "InputManager.h"

#include "ControllerRegistry.h"
#include "SceneRegistry.h"

#include "CameraController.h"

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
    const WCHAR *physicsFileName = nullptr;
#ifdef _DEBUG
    // rendererFileName = L"../../DLL/RendererD3D12_x64_debug.dll";
    rendererFileName = L"../../DLL/RendererRaytracing_x64_Debug.dll";
    engineFileName = L"../../DLL/EngineModule_x64_Debug.dll";
    exporterFileName = L"../../DLL/ModelExporter_x64_Debug.dll";
    physicsFileName = L"../../DLL/PhysicsModule_x64_Debug.dll";
#else
    rendererFileName = L"../../DLL/RendererRaytracing_x64_Release.dll";
    engineFileName = L"../../DLL/EngineModule_x64_Release.dll";
    exporterFileName = L"../../DLL/ModelExporter_x64_Release.dll";
    physicsFileName = L"../../DLL/PhysicsModule_x64_Release.dll";
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

    m_hPhysicsDLL = LoadLibrary(physicsFileName);
    if (!m_hPhysicsDLL)
    {
        dwErrCode = GetLastError();
        swprintf_s(wchErrTxt, L"Fail to LoadLibrary(%s) - Error Code: %u", physicsFileName, dwErrCode);
        MessageBox(hWnd, wchErrTxt, L"Error", MB_OK);
        __debugbreak();
    }
    pCreateFunc = (CREATE_INSTANCE_FUNC)GetProcAddress(m_hPhysicsDLL, "DllCreateInstance");
    pCreateFunc(&m_pPhysics);

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
    result = m_pPhysics->Initialize();
    result = m_pGame->Initialize(hWnd, m_pRenderer, m_pPhysics);
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

    SceneRegistry::GetInstance().UnLoad();

    CleanupControllers();

    CleanupModules();
}

BOOL Client::Initialize(HWND hWnd)
{
    BOOL result = FALSE;

    float f[12];
    for (int i = 0; i < 12; i++)
    {
        f[i] = 1.0f;
    }
    Vector12 v1(f);

    for (int i = 0; i < 12; i++)
    {
        f[i] = 1.0f;
    }
    Vector12 v2(f);

    float x = v1.Dot(v2);

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

void Client::LoadScene() { SceneRegistry::GetInstance().ChangeScene("PhysicsDemoController"); }

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

        SceneRegistry::GetInstance().Update(dt);

        m_pGame->Update(dt);

        m_pInputManager->Update();
        m_prevWindowResized = m_windowResized;
        m_windowResized = FALSE;
    }

    m_pGame->Render();

    // 성능 측정
    m_frameCount++;
    if (curTick - m_prevFrameCheckTick > 1000)
    {
        m_prevFrameCheckTick = curTick;
        m_FPS = m_frameCount;
        m_frameCount = 0;

        WCHAR text[64];
        wsprintf(text, L"FrameRate: %d", m_FPS);
        SetWindowText(m_hWnd, text);
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

    m_windowResized = TRUE;

    return m_pGame->OnUpdateWindowSize(width, height);
}

Client::~Client() { Cleanup(); }
