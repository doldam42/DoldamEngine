#include "pch.h"

#include <filesystem>

#include "../EngineModule/EngineInterface.h"
#include "../RenderModule/RendererInterface.h"

#include "AudioManager.h"
#include "VideoManager.h"

#include "BadAppleController.h"
#include "RaytracingDemoController.h"
#include "TessellationDemoController.h"
#include "TimeController.h"

#include "Client.h"

namespace fs = std::filesystem;

Client *g_pClient = nullptr;

void Client::CleanupControllers()
{
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
    CleanupControllers();
    if (m_pFbxExporter)
    {
        DeleteFbxExporter(m_pFbxExporter);
        m_pFbxExporter = nullptr;
    }
    if (m_pAssimpExporter)
    {
        DeleteAssimpExporter(m_pAssimpExporter);
        m_pAssimpExporter = nullptr;
    }
    if (m_pAudio)
    {
        delete m_pAudio;
        m_pAudio = nullptr;
    }
    if (m_pGame)
    {
        DeleteGameEngine(m_pGame);
        m_pGame = nullptr;
    }
}

BOOL Client::Initialize(HWND hWnd)
{
    BOOL result = FALSE;
    result = CreateGameEngine(hWnd, &m_pGame);

    m_pRenderer = m_pGame->GetRenderer();
    result = CreateFbxExporter(&m_pFbxExporter);
    result = CreateAssimpExporter(&m_pAssimpExporter);
    result = m_pFbxExporter->Initialize(m_pGame);
    result = m_pAssimpExporter->Initialize(m_pGame);

    m_pAudio = new AudioManager;
    m_pAudio->Initialize();

    // Register Controllers Before Start Game Manager.
    m_pTimeController = new TimeController;
    //m_pRaytracingDemoController = new RaytracingDemoController;
    m_pTessellationDemoController = new TessellationDemoController();

    m_pGame->Register(m_pAudio);
    m_pGame->Register(m_pTimeController);
    //m_pGame->Register(m_pRaytracingDemoController);
    m_pGame->Register(m_pTessellationDemoController);

    Start();
    m_pGame->Start();

    return result;
}

void Client::LoadResources()
{

}

void Client::LoadScene() 
{

}

void Client::Process()
{
    m_pGame->Update();

    m_pGame->Render();
}

BOOL Client::Start() 
{
    LoadResources();

    LoadScene();
    
    m_pGame->BuildScene();

    return TRUE;
}

void Client::OnKeyDown(UINT nChar, UINT uiScanCode) { m_pGame->OnKeyDown(nChar, uiScanCode); }

void Client::OnKeyUp(UINT nChar, UINT uiScanCode) { m_pGame->OnKeyUp(nChar, uiScanCode); }

void Client::OnMouseMove(int mouseX, int mouseY) { m_pGame->OnMouseMove(mouseX, mouseY); }

void Client::OnMouseWheel(float deltaWheel) { m_pGame->OnMouseWheel(deltaWheel); }

BOOL Client::OnUpdateWindowSize(UINT width, UINT height) { return m_pGame->OnUpdateWindowSize(width, height); }

Client::~Client() { Cleanup(); }
