#include "pch.h"

#include "GUIController.h"

#include "GameEditor.h"

BOOL GameEditor::LoadModules(HWND hWnd) 
{
    BOOL                 result = FALSE;
    CREATE_INSTANCE_FUNC pCreateFunc;

    const WCHAR *rendererFileName = nullptr;
    const WCHAR *engineFileName = nullptr;
    const WCHAR *exporterFileName = nullptr;
#ifdef _DEBUG
    // rendererFileName = L"../../DLL/RendererD3D12_x64_debug.dll";
    rendererFileName = L"../../DLL/RendererRaytracing_x64_debug.dll";
    engineFileName = L"../../DLL/EngineModule_x64_debug.dll";
#else
    rendererFileName = L"../../DLL/RendererD3D12_x64_release.dll";
    engineFileName = L"../../DLL/EngineModule_x64_release.dll";
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

void GameEditor::Cleanup()
{
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

    LoadModules(hWnd);

    m_pGUIController = new GUIController;
    m_pGUIController->Initilize(m_pRenderer->GetRenderGUI());
    m_pGame->Register(m_pGUIController);

    m_hWnd = hWnd;
    return TRUE;
}

void GameEditor::Process() 
{ 
    m_pGame->Update();

    m_pGame->Render();
}

LRESULT GameEditor::WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return m_pRenderer->GetRenderGUI()->WndProcHandler(hWnd, msg, wParam, lParam);
}

GameEditor::~GameEditor() { Cleanup(); }
