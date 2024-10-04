#include "pch.h"

#include "AnimationClip.h"
#include "Camera.h"
#include "Character.h"
#include "GameObject.h"
#include "GeometryGenerator.h"
#include "Model.h"
#include "Sprite.h"

#include "GameEngine.h"

GameEngine *g_pGame = nullptr;

UINT GameEngine::initRefCount = 0;

Model *GameEngine::SquareMesh = nullptr;
Model *GameEngine::BoxMesh = nullptr;
Model *GameEngine::SphereMesh = nullptr;

void GameEngine::LoadPrimitiveMeshes()
{
    if (!BoxMesh)
    {
        BoxMesh = GeometryGenerator::MakeBox();
        BoxMesh->InitMeshHandles(m_pRenderer);
    }
    if (!SquareMesh)
    {
        SquareMesh = GeometryGenerator::MakeSquare();
        SquareMesh->InitMeshHandles(m_pRenderer);
    }
}

void GameEngine::DeletePrimitiveMeshes()
{
    if (BoxMesh)
    {
        if (BoxMesh->Release() != 0)
        {
            __debugbreak();
        }
        BoxMesh = nullptr;
    }
    if (SquareMesh)
    {
        if (SquareMesh->Release() != 0)
        {
            __debugbreak();
        }
        SquareMesh = nullptr;
    }
}

void GameEngine::Cleanup()
{
    DeleteAllSprite();

    DeleteAllGameObject();

    DeletePrimitiveMeshes();

    DeleteAllModel();

    DeleteAllAnimation();

    if (m_pRenderer)
    {
        DeleteD3D12Renderer(m_pRenderer);
        m_pRenderer = nullptr;
    }
    if (m_pMainCamera)
    {
        delete m_pMainCamera;
        m_pMainCamera = nullptr;
    }
    if (m_pInputManager)
    {
        delete m_pInputManager;
        m_pInputManager = nullptr;
    }
    if (m_pTimer)
    {
        delete m_pTimer;
        m_pTimer = nullptr;
    }
    if (m_pPerformanceTimer)
    {
        delete m_pPerformanceTimer;
        m_pPerformanceTimer = nullptr;
    }
}

BOOL GameEngine::Initialize(HWND hWnd)
{
    BOOL result = FALSE;

    RECT rect;
    GetClientRect(hWnd, &rect);
    DWORD width = rect.right - rect.left;
    DWORD height = rect.bottom - rect.top;

    m_pInputManager = new InputManager();
    m_pInputManager->SetWindowSize(width, height);

    m_pTimer = new Timer;
    m_pPerformanceTimer = new Timer;

    if (!CreateD3D12Renderer(hWnd, TRUE, FALSE, &m_pRenderer))
    {
        __debugbreak();
        goto lb_return;
    }

    m_renderThreadCount = m_pRenderer->GetRenderThreadCount();

    m_pMainCamera = new Camera;
    m_pMainCamera->SetAspectRatio(m_pRenderer->GetAspectRatio());
    m_pMainCamera->m_useFirstPersonView = true;

    m_hWnd = hWnd;

    m_pAnimationHashTable = new HashTable();
    m_pAnimationHashTable->Initialize(13, MAX_NAME, 128); // TODO: 최적의 버킷 개수 정하기

    LoadPrimitiveMeshes();
    LoadResources();

    result = TRUE;
lb_return:
    m_pTimer->Tick();
    m_pPerformanceTimer->Tick();
    return result;
}

void GameEngine::LoadResources()
{
    // Create Cubemap
    m_pRenderer->InitCubemaps(
        L"..\\..\\assets\\sponza\\env\\cubemapEnvHDR.dds", L"..\\..\\assets\\sponza\\env\\cubemapSpecularHDR.dds",
        L"..\\..\\assets\\sponza\\env\\cubemapDiffuseHDR.dds", L"..\\..\\assets\\sponza\\env\\cubemapBrdf.dds");

    // Create Lights
    Vector3 radiance = Vector3(1.0f);
    Vector3 direction = Vector3(0.0f, 0.0f, 1.0f);
    m_pLight = m_pRenderer->CreateDirectionalLight(&radiance, &direction);
}

void GameEngine::OnKeyDown(UINT nChar, UINT uiScanCode) { m_pInputManager->OnKeyDown(nChar, uiScanCode); }

void GameEngine::OnKeyUp(UINT nChar, UINT uiScanCode) { m_pInputManager->OnKeyUp(nChar, uiScanCode); }

void GameEngine::OnMouseMove(int mouseX, int mouseY) { m_pInputManager->OnMouseMove(mouseX, mouseY); }

void GameEngine::ProcessInput()
{
    if (m_pInputManager->IsKeyPressed(VK_ESCAPE))
    {
        DestroyWindow(m_hWnd);
    }
}

void GameEngine::PreUpdate(ULONGLONG curTick) { ProcessInput(); }

void GameEngine::Update(ULONGLONG curTick)
{
    // Update Scene with 60FPS
    if (curTick - m_prevUpdateTick < 23)
    {
        return;
    }
    float dt = static_cast<float>(curTick - m_prevUpdateTick) / 1000.f;
    m_prevUpdateTick = curTick;

    // camera
    if (m_activateCamera)
    {
        m_pMainCamera->Update(dt);
    }
    // update objects

    SORT_LINK *pCur = m_pGameObjLinkHead;
    while (pCur)
    {
        GameObject *pGameObj = (GameObject *)pCur->pItem;
        pGameObj->Run();
        pCur = pCur->pNext;
    }

    LateUpdate(dt);
}

void GameEngine::LateUpdate(ULONGLONG curTick)
{
    //// m_pMainCharacter->LateUpdate();
    // for (int i = 0; i < m_scene.size(); i++)
    //{
    //     m_scene[i]->LateUpdate(dt);
    // }
}

void GameEngine::Render()
{
    m_pRenderer->UpdateCamera(m_pMainCamera->At(), m_pMainCamera->GetViewRow(), m_pMainCamera->GetProjRow());

    // begin
    m_pRenderer->BeginRender();

    // render game objects
    SORT_LINK *pCur = m_pGameObjLinkHead;
    UINT       objCount = 0;
    while (pCur)
    {
        GameObject *pGameObj = (GameObject *)pCur->pItem;
        pGameObj->Render();
        pCur = pCur->pNext;
        objCount++;
    }

    // render sprites
    pCur = m_pSpriteLinkHead;
    UINT spriteCount = 0;
    while (pCur)
    {
        Sprite *pSprite = (Sprite *)pCur->pItem;
        pSprite->Render();
        pCur = pCur->pNext;
        spriteCount++;
    }

    //// render dynamic texture sprite
    // m_pRenderer->RenderSprite(m_pSprite, 512 + 10, 0, 0.5f, 0.5f, 1.0f);
    // m_pRenderer->RenderSpriteWithTex(m_pSpriteCommon, 0, 256 + 5 + 256 + 5, 1.0f, 1.0f, nullptr, 0.0f,
    //                                  m_pDynamicTextureHandle);

    //// render dynamic texture as text
    // m_pRenderer->RenderSpriteWithTex(m_pSpriteCommon, 512 + 5, 256 + 5 + 256 + 5, 1.0f, 1.0f, nullptr, 0.0f,
    //                                  m_pTextTexHandle);
    m_pRenderer->EndRender();
    m_pRenderer->Present();
}

BOOL GameEngine::OnUpdateWindowSize(UINT width, UINT height)
{
    m_pInputManager->SetWindowSize(width, height);
    m_pMainCamera->SetAspectRatio(float(width) / height);

    if (m_pRenderer)
    {
        m_pRenderer->OnUpdateWindowSize(width, height);
    }
    return TRUE;
}

IGameObject *GameEngine::CreateGameObject()
{
    GameObject *pGameObj = new GameObject;
    pGameObj->Initialize(this);
    LinkToLinkedListFIFO(&m_pGameObjLinkHead, &m_pGameObjLinkTail, &pGameObj->m_LinkInGame);

    return pGameObj;
}

void GameEngine::DeleteGameObject(IGameObject *pGameObj)
{
    GameObject *pObj = (GameObject *)pGameObj;
    UnLinkFromLinkedList(&m_pGameObjLinkHead, &m_pGameObjLinkTail, &pObj->m_LinkInGame);
    delete pObj;
}

void GameEngine::DeleteAllGameObject()
{
    while (m_pGameObjLinkHead)
    {
        GameObject *pGameObj = (GameObject *)(m_pGameObjLinkHead->pItem);
        DeleteGameObject(pGameObj);
    }
}

IGameModel *GameEngine::GetPrimitiveModel(PRIMITIVE_MODEL_TYPE type)
{
    switch (type)
    {
    case PRIMITIVE_MODEL_TYPE_SQUARE:
        return SquareMesh;
    case PRIMITIVE_MODEL_TYPE_BOX:
        return BoxMesh;
    default:
        break;
    }
    return nullptr;
}

IGameModel *GameEngine::CreateModelFromFile(const WCHAR *basePath, const WCHAR *filename)
{
    Model *pModel = GeometryGenerator::ReadFromFile(basePath, filename);
    pModel->InitMeshHandles(m_pRenderer);

    LinkToLinkedListFIFO(&m_pModelLinkHead, &m_pModelLinkTail, &pModel->m_LinkInGame);
    pModel->AddRef();
    return pModel;
}

void GameEngine::DeleteModel(IGameModel *pModel)
{
    Model *pM = (Model *)pModel;
    UnLinkFromLinkedList(&m_pModelLinkHead, &m_pModelLinkTail, &pM->m_LinkInGame);
    delete pM;
}

void GameEngine::DeleteAllModel()
{
    while (m_pModelLinkHead)
    {
        Model *pModel = (Model *)(m_pModelLinkHead->pItem);
        DeleteModel(pModel);
    }
}

IGameSprite *GameEngine::CreateSpriteFromFile(const WCHAR *basePath, const WCHAR *filename, UINT width, UINT height)
{
    WCHAR path[MAX_PATH] = {L'\0'};
    wcscpy_s(path, basePath);
    wcscat_s(path, filename);

    Sprite *pSprite = new Sprite;
    pSprite->Initialize(m_pRenderer, path, width, height);
    LinkToLinkedListFIFO(&m_pSpriteLinkHead, &m_pSpriteLinkTail, &pSprite->m_LinkInGame);
    return pSprite;
}

IGameSprite *GameEngine::CreateDynamicSprite(UINT width, UINT height)
{
    DynamicSprite *pSprite = new DynamicSprite;
    pSprite->Initialize(m_pRenderer, width, height);
    LinkToLinkedListFIFO(&m_pSpriteLinkHead, &m_pSpriteLinkTail, &pSprite->m_LinkInGame);
    return pSprite;
}

void GameEngine::DeleteSprite(IGameSprite *pSprite)
{
    Sprite *pS = (Sprite *)pSprite;
    UnLinkFromLinkedList(&m_pSpriteLinkHead, &m_pSpriteLinkTail, &pS->m_LinkInGame);
    delete pS;
}

void GameEngine::DeleteAllSprite()
{
    while (m_pSpriteLinkHead)
    {
        Sprite *pSprite = (Sprite *)(m_pSpriteLinkHead->pItem);
        DeleteSprite(pSprite);
    }
}

IGameAnimation *GameEngine::CreateAnimationFromFile(const WCHAR *basePath, const WCHAR *filename)
{
    AnimationClip *pClip = nullptr;
    UINT keySize = wcslen(filename) * sizeof(WCHAR);

    if (m_pAnimationHashTable->Select((void**)&pClip, 1, filename, keySize))
    {
        pClip->AddRef();
    }
    else
    {
        AnimationClip *pClip = GeometryGenerator::ReadAnimationFromFile(basePath, filename);

        pClip->m_pSearchHandleInGame = m_pAnimationHashTable->Insert((void *)pClip, filename, keySize);
    }

    return pClip;
}

void GameEngine::DeleteAnimation(IGameAnimation *pInAnim)
{
    AnimationClip *pAnim = dynamic_cast<AnimationClip *>(pInAnim);
    if (!pAnim->ref_count)
        __debugbreak;

    LONG refCount = --pAnim->ref_count;
    if (!refCount)
    {
        m_pAnimationHashTable->Delete(pAnim->m_pSearchHandleInGame);
        pAnim->m_pSearchHandleInGame = nullptr;

        delete pAnim;
    }
}

void GameEngine::DeleteAllAnimation() 
{
    if (m_pAnimationHashTable)
    {
        m_pAnimationHashTable->DeleteAll();
        delete m_pAnimationHashTable;
        m_pAnimationHashTable = nullptr;
    }
}

void GameEngine::ToggleCamera() { m_activateCamera = !m_activateCamera; }

GameEngine::~GameEngine() { Cleanup(); }
