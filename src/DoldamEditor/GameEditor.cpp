#include "pch.h"

#include "CameraController.h"
#include "InputManager.h"
#include "GUIController.h"

#include "GameEditor.h"

GameEditor* g_pEditor = nullptr;

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
    //rendererFileName = L"../../DLL/RendererD3D12_x64_Release.dll";
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

    result = m_pRenderer->Initialize(hWnd, TRUE, FALSE);
    result = m_pGame->Initialize(hWnd, m_pRenderer);

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

    m_pInputManager = new InputManager;
    m_pInputManager->Initialize(width, height);

    m_pGUIController = new GUIController;
    m_pGUIController->Initilize(m_pRenderer->GetRenderGUI(), m_assetPath);
    m_pGame->Register(m_pGUIController);
    
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
    m_pInputManager->SetWindowSize(width, height);
    return m_pGame->OnUpdateWindowSize(width, height);
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
