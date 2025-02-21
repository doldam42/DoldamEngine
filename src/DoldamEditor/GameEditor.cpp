#include "pch.h"

#include "CameraController.h"
#include "FileManager.h"
#include "GUIController.h"
#include "GUIView.h"
#include "InputManager.h"

#include "GameEditor.h"

GameEditor *g_pEditor = nullptr;

BOOL GameEditor::LoadModules(HWND hWnd)
{
    BOOL                 result = FALSE;
    CREATE_INSTANCE_FUNC pCreateFunc;

    const WCHAR *rendererFileName = nullptr;
    const WCHAR *engineFileName = nullptr;
#ifdef _DEBUG
    // rendererFileName = L"../../DLL/RendererD3D12_x64_Debug.dll";
    rendererFileName = L"../../DLL/RendererRaytracing_x64_Debug.dll";
    engineFileName = L"../../DLL/EngineModule_x64_Debug.dll";
#else
    rendererFileName = L"../../DLL/RendererRaytracing_x64_Release.dll";
    // rendererFileName = L"../../DLL/RendererD3D12_x64_Release.dll";
    engineFileName = L"../../DLL/EngineModule_x64_Release.dll";
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

    RECT rect;
    GetClientRect(hWnd, &rect);
    DWORD width = rect.right - rect.left;
    DWORD height = rect.bottom - rect.top;
    UINT  viewportWidth = width * GUIView::SCENE_VIEW_WIDTH;
    UINT  viewportHeight = height * GUIView::SCENE_VIEW_HEIGHT;

    result = m_pRenderer->Initialize(hWnd, TRUE, FALSE, TRUE, viewportWidth, viewportHeight);
    result = m_pGame->Initialize(hWnd, m_pRenderer, TRUE, viewportWidth, viewportHeight);

    if (!result)
        __debugbreak();

    return result;
}

void GameEditor::CleanupModules()
{
    if (m_pGame)
    {
        m_pGame->Release();
        m_pGame = nullptr;
    }
    if (m_pRenderer)
    {
        m_pRenderer->Release();
        m_pRenderer = nullptr;
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

BOOL GameEditor::LoadConfigs()
{
    GetPrivateProfileString(L"path", L"asset_path", L"\0", m_assetPath, MAX_PATH, L".\\Config.ini");
    if (!wcslen(m_assetPath))
    {
        __debugbreak();
        return FALSE;
    }
    return TRUE;
}

void GameEditor::ProcessInput()
{
    if (m_pInputManager->IsKeyPressed(VK_ESCAPE))
    {
        DestroyWindow(m_hWnd);
    }
}

void GameEditor::Cleanup()
{
    if (m_pCameraController)
    {
        delete m_pCameraController;
        m_pCameraController = nullptr;
    }
    if (m_pInputManager)
    {
        delete m_pInputManager;
        m_pInputManager = nullptr;
    }
    if (m_pGUIController)
    {
        delete m_pGUIController;
        m_pGUIController = nullptr;
    }
    if (m_pFileManager)
    {
        delete m_pFileManager;
        m_pFileManager = nullptr;
    }
    CleanupModules();
}

BOOL GameEditor::Initialize(HWND hWnd)
{
    // Show the window
    ::ShowWindow(hWnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hWnd);

    LoadConfigs();

    LoadModules(hWnd);

    RECT rect;
    GetClientRect(hWnd, &rect);
    DWORD width = rect.right - rect.left;
    DWORD height = rect.bottom - rect.top;

    m_pFileManager = new FileManager;
    m_pFileManager->Initialize(m_assetPath);

    m_pGUIController = new GUIController;
    m_pGUIController->Initialize(m_pRenderer, m_pFileManager->GetRootDir(), m_pFileManager->GetBasePath(), width, height);
    m_pGame->Register(m_pGUIController);

    UINT viewportWidth = width * GUIView::SCENE_VIEW_WIDTH;
    UINT viewportHeight = height * GUIView::SCENE_VIEW_HEIGHT;

    m_pInputManager = new InputManager;
    m_pInputManager->Initialize(viewportWidth, viewportHeight);

    m_pCameraController = new CameraController;
    m_pCameraController->Initialize(this);
    m_pGame->Register(m_pCameraController);

    m_pGame->Start();

    m_hWnd = hWnd;
    return TRUE;
}

void GameEditor::Process()
{

    ProcessInput();

    m_pGame->Update();

    m_pGame->Render();
}

void GameEditor::OnKeyDown(UINT nChar, UINT uiScanCode) { m_pInputManager->OnKeyDown(nChar, uiScanCode); }

void GameEditor::OnKeyUp(UINT nChar, UINT uiScanCode) { m_pInputManager->OnKeyUp(nChar, uiScanCode); }

void GameEditor::OnMouseMove(int mouseX, int mouseY) { m_pInputManager->OnMouseMove(mouseX, mouseY); }

void GameEditor::OnMouseWheel(float deltaWheel) { m_pInputManager->OnMouseWheel(deltaWheel); }

BOOL GameEditor::OnUpdateWindowSize(UINT width, UINT height)
{
    Vector2 viewportSizeR = m_pGUIController->GetViewportSizeRatio();
    UINT    viewportWidth = width * viewportSizeR.x;
    UINT    viewportHeight = height * viewportSizeR.y;

    m_pGUIController->OnUpdateWindowSize(width, height);
    m_pInputManager->SetWindowSize(viewportWidth, viewportHeight);
    
    return m_pGame->OnUpdateWindowSize(width, height, viewportWidth, viewportHeight);
}

LRESULT GameEditor::WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (m_pRenderer)
    {
        return m_pRenderer->GetRenderGUI()->WndProcHandler(hWnd, msg, wParam, lParam);
    }
    return 0;
}

GameEditor::~GameEditor() { Cleanup(); }
