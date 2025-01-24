#include "pch.h"

#include "AnimationClip.h"
#include "Camera.h"
#include "Character.h"
#include "ControllerManager.h"
#include "GameObject.h"
#include "GeometryGenerator.h"
#include "Model.h"
#include "PhysicsManager.h"
#include "Sprite.h"
#include "World.h"

#include "GameManager.h"

GameManager *g_pGame = nullptr;

UINT GameManager::initRefCount = 0;

Model *GameManager::SquareMesh = nullptr;
Model *GameManager::BoxMesh = nullptr;
Model *GameManager::SphereMesh = nullptr;

void GameManager::LoadPrimitiveMeshes()
{
    if (!BoxMesh)
    {
        BoxMesh = GeometryGenerator::MakeBox();
        BoxMesh->InitRenderComponents(m_pRenderer);
    }
    if (!SquareMesh)
    {
        SquareMesh = GeometryGenerator::MakeSquare();
        SquareMesh->InitRenderComponents(m_pRenderer);
    }
    if (!SphereMesh)
    {
        SphereMesh = GeometryGenerator::MakeSphere(1.0f, 32, 32);
        SphereMesh->InitRenderComponents(m_pRenderer);
    }
}

void GameManager::DeletePrimitiveMeshes()
{
    if (BoxMesh)
    {
        delete BoxMesh;
        BoxMesh = nullptr;
    }
    if (SquareMesh)
    {
        delete SquareMesh;
        SquareMesh = nullptr;
    }
    if (SphereMesh)
    {
        delete SphereMesh;
        SphereMesh = nullptr;
    }
}

void GameManager::Cleanup()
{
    /*if (m_pShadowMapSprite)
    {
        m_pShadowMapSprite->Release();
        m_pShadowMapSprite = nullptr;
    }*/

    if (m_pWorld)
    {
        delete m_pWorld;
        m_pWorld = nullptr;
    }
    if (m_pTerrain)
    {
        m_pTerrain->Release();
        m_pTerrain = nullptr;
    }

    DeleteAllSprite();

    DeleteAllGameObject();

    DeletePrimitiveMeshes();

    DeleteAllModel();

    DeleteAllAnimation();

    if (m_pControllerManager)
    {
        delete m_pControllerManager;
    }

    if (m_pPhysicsManager)
    {
        delete m_pPhysicsManager;
        m_pPhysicsManager = nullptr;
    }

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
}

BOOL GameManager::Initialize(HWND hWnd)
{
    BOOL result = FALSE;

    RECT rect;
    GetClientRect(hWnd, &rect);
    DWORD width = rect.right - rect.left;
    DWORD height = rect.bottom - rect.top;

    g_pGame = this;

    m_pInputManager = new InputManager();
    m_pInputManager->Initialize(width, height);

    if (!CreateD3D12Renderer(hWnd, TRUE, FALSE, &m_pRenderer))
    {
        __debugbreak();
        goto lb_return;
    }

    m_renderThreadCount = m_pRenderer->GetRenderThreadCount();

    m_pPhysicsManager = new PhysicsManager;
    m_pPhysicsManager->Initialize();

    m_pMainCamera = new CameraController;
    m_pMainCamera->Initialize(XMConvertToRadians(90.0f), static_cast<float>(width) / height, 0.01f, 1000.0f);
    m_pMainCamera->m_useFirstPersonView = true;

    m_hWnd = hWnd;

    m_pAnimationHashTable = new HashTable();
    m_pAnimationHashTable->Initialize(13, MAX_NAME, 128); // TODO: 최적의 버킷 개수 정하기

    m_pControllerManager = new ControllerManager;

    m_pWorld = new World;
    m_pWorld->Initialize();

    result = TRUE;
lb_return:
    return result;
}

BOOL GameManager::LoadResources()
{
    // Create Shadow Map Sprite
    //m_pShadowMapSprite = m_pRenderer->CreateSpriteObject();

    // Create Cubemap
    m_pRenderer->InitCubemaps(L"..\\..\\assets\\textures\\Skybox\\DaySky\\DaySkyEnvHDR.dds",
                              L"..\\..\\assets\\textures\\Skybox\\DaySky\\DaySkySpecularHDR.dds",
                              L"..\\..\\assets\\textures\\Skybox\\DaySky\\DaySkyDiffuseHDR.dds",
                              L"..\\..\\assets\\textures\\Skybox\\DaySky\\DaySkyBrdf.dds");

    // Create Lights
    Vector3 radiance = Vector3(1.0f);
    Vector3 direction = Vector3(0.832f, -0.554f, 0.0f);
    Vector3 position = Vector3(-16.0f, 16.0f, 0.0f);

    direction.Normalize();
    // m_pLight = m_pRenderer->CreateSpotLight(&radiance, &direction, &position, 0.5f, 0.35);
    // m_pLight = m_pRenderer->CreatePointLight(&radiance, &direction, &position, 0.35f);
    m_pLight = m_pRenderer->CreateDirectionalLight(&radiance, &direction, &position);

    return TRUE;
}

void GameManager::OnKeyDown(UINT nChar, UINT uiScanCode) { m_pInputManager->OnKeyDown(nChar, uiScanCode); }

void GameManager::OnKeyUp(UINT nChar, UINT uiScanCode) { m_pInputManager->OnKeyUp(nChar, uiScanCode); }

void GameManager::OnMouseMove(int mouseX, int mouseY) { m_pInputManager->OnMouseMove(mouseX, mouseY); }

void GameManager::ProcessInput()
{
    if (m_pInputManager->IsKeyPressed(VK_ESCAPE))
    {
        DestroyWindow(m_hWnd);
    }
}

void GameManager::Start()
{
    ULONGLONG prevTick = GetTickCount64();

    LoadPrimitiveMeshes();

    LoadResources();

    m_pControllerManager->Start();

    ULONGLONG curTick = GetTickCount64();
    m_loadingTime = static_cast<float>(curTick - prevTick) / 1000.f;
    m_prevUpdateTick = curTick;
    m_prevFrameCheckTick = curTick;
}

void GameManager::Update()
{
    ULONGLONG curTick = GetTickCount64();
    PreUpdate(curTick);
    Update(curTick);
    LateUpdate(curTick);
}

void GameManager::BuildScene()
{
    m_pWorld->BeginCreateWorld(MAX_WORLD_OBJECT_COUNT);

    SORT_LINK *pCur = m_pGameObjLinkHead;
    while (pCur)
    {
        GameObject *pObj = (GameObject *)pCur->pItem;
        m_pWorld->InsertObject(pObj);
        pCur = pCur->pNext;
    }

    m_pWorld->EndCreateWorld();
}

void GameManager::PreUpdate(ULONGLONG curTick) { ProcessInput(); }

void GameManager::UpdatePhysics(float dt)
{
    SORT_LINK *pCur = m_pGameObjLinkHead;
    while (pCur)
    {
        GameObject       *pGameObj = (GameObject *)pCur->pItem;
        PhysicsComponent *pComp = pGameObj->GetPhysicsComponent();
        if (pComp)
        {
            pComp->ApplyGravityImpulse(dt);
        }

        pCur = pCur->pNext;
    }

    pCur = m_pGameObjLinkHead;
    while (pCur)
    {
        GameObject *pGameObj = (GameObject *)pCur->pItem;

        if (pGameObj->GetPhysicsComponent())
        {
            m_pPhysicsManager->CollisionTest(pGameObj, dt);
        }

        pCur = pCur->pNext;
    }

    m_pPhysicsManager->ResolveContactsAll(dt);
}

void GameManager::Update(ULONGLONG curTick)
{
    // Update Scene with 40FPS
    if (curTick - m_prevUpdateTick < 25)
    {
        return;
    }
    float dt = static_cast<float>(curTick - m_prevUpdateTick) / 1000.f;
    m_prevUpdateTick = curTick;
    // FPS가 너무 낮은 경우 30 FPS로 고정
    if (dt > 0.1f) // 10 FPS
    {
        dt = 0.03f; // 30 FPS
    }
    
    // Camera Update
    if (m_activateCamera)
    {
        m_pMainCamera->Update(dt);
    }

    if (m_isPaused)
    {
        return;
    }
    dt *= m_timeSpeed;

    m_deltaTime = dt;

    // Update Controller
    m_pControllerManager->Update(dt);

    UpdatePhysics(dt);

    // Update Game Objects
    SORT_LINK *pCur = m_pGameObjLinkHead;
    while (pCur)
    {
        GameObject *pGameObj = (GameObject *)pCur->pItem;

        pGameObj->Update(dt);

        pCur = pCur->pNext;
    }
}

void GameManager::LateUpdate(ULONGLONG curTick)
{
    // 성능 측정
    m_frameCount++;
    if (curTick - m_prevFrameCheckTick > 1000)
    {
        m_prevFrameCheckTick = curTick;
        m_FPS = m_frameCount;
        m_frameCount = 0;
    }
}

void GameManager::Render()
{
    m_pRenderer->UpdateCamera(m_pMainCamera->Eye(), m_pMainCamera->GetViewRow(), m_pMainCamera->GetProjRow());

    // begin
    m_pRenderer->BeginRender();

    m_pRenderer->RenderTerrain(m_pTerrain);

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
    /*m_pRenderer->RenderSpriteWithTex(m_pShadowMapSprite, 0, 0, 0.25f, 0.25f, nullptr, 0,
                                     m_pRenderer->GetShadowMapTexture(0));*/
    m_pRenderer->EndRender();
    m_pRenderer->Present();
}

BOOL GameManager::OnUpdateWindowSize(UINT width, UINT height)
{
    m_pInputManager->SetWindowSize(width, height);
    m_pMainCamera->SetAspectRatio(float(width) / height);

    if (m_pRenderer)
    {
        m_pRenderer->OnUpdateWindowSize(width, height);
    }
    return TRUE;
}

void GameManager::OnMouseWheel(float deltaWheel) { m_pInputManager->OnMouseWheel(deltaWheel); }

IGameCharacter *GameManager::CreateCharacter()
{
    Character *pGameObj = new Character;
    pGameObj->Initialize(this, 5);

    LinkToLinkedListFIFO(&m_pGameObjLinkHead, &m_pGameObjLinkTail, &pGameObj->m_LinkInGame);

    return pGameObj;
}

IGameObject *GameManager::CreateGameObject()
{
    GameObject *pGameObj = new GameObject;
    pGameObj->Initialize(this);
    LinkToLinkedListFIFO(&m_pGameObjLinkHead, &m_pGameObjLinkTail, &pGameObj->m_LinkInGame);

    return pGameObj;
}

void GameManager::DeleteGameObject(IGameObject *pGameObj)
{
    GameObject *pObj = (GameObject *)pGameObj;
    UnLinkFromLinkedList(&m_pGameObjLinkHead, &m_pGameObjLinkTail, &pObj->m_LinkInGame);
    delete pObj;
}

void GameManager::DeleteAllGameObject()
{
    while (m_pGameObjLinkHead)
    {
        GameObject *pGameObj = (GameObject *)(m_pGameObjLinkHead->pItem);
        DeleteGameObject(pGameObj);
    }
}

IGameModel *GameManager::GetPrimitiveModel(PRIMITIVE_MODEL_TYPE type)
{
    switch (type)
    {
    case PRIMITIVE_MODEL_TYPE_SQUARE:
        SquareMesh->AddRef();
        return SquareMesh;
    case PRIMITIVE_MODEL_TYPE_BOX:
        BoxMesh->AddRef();
        return BoxMesh;
    case PRIMITIVE_MODEL_TYPE_SPHERE:
        SphereMesh->AddRef();
        return SphereMesh;
    default:
        break;
    }
    return nullptr;
}

IGameModel *GameManager::CreateModelFromFile(const WCHAR *basePath, const WCHAR *filename)
{
    Model *pModel = GeometryGenerator::ReadFromFile(basePath, filename);
    pModel->InitRenderComponents(m_pRenderer);

    LinkToLinkedListFIFO(&m_pModelLinkHead, &m_pModelLinkTail, &pModel->m_LinkInGame);
    pModel->AddRef();
    return pModel;
}

IGameModel *GameManager::CreateEmptyModel()
{
    Model *pModel = new Model;

    LinkToLinkedListFIFO(&m_pModelLinkHead, &m_pModelLinkTail, &pModel->m_LinkInGame);
    pModel->AddRef();
    return pModel;
}

void GameManager::DeleteModel(IGameModel *pModel)
{
    Model *pM = (Model *)pModel;
    UnLinkFromLinkedList(&m_pModelLinkHead, &m_pModelLinkTail, &pM->m_LinkInGame);
    delete pM;
}

void GameManager::DeleteAllModel()
{
    while (m_pModelLinkHead)
    {
        Model *pModel = (Model *)(m_pModelLinkHead->pItem);
        DeleteModel(pModel);
    }
}

IGameSprite *GameManager::CreateSpriteFromFile(const WCHAR *basePath, const WCHAR *filename, UINT width, UINT height)
{
    WCHAR path[MAX_PATH] = {L'\0'};
    wcscpy_s(path, basePath);
    wcscat_s(path, filename);

    Sprite *pSprite = new Sprite;
    pSprite->Initialize(m_pRenderer, path, width, height);
    LinkToLinkedListFIFO(&m_pSpriteLinkHead, &m_pSpriteLinkTail, &pSprite->m_LinkInGame);
    return pSprite;
}

IGameSprite *GameManager::CreateDynamicSprite(UINT width, UINT height)
{
    DynamicSprite *pSprite = new DynamicSprite;
    pSprite->Initialize(m_pRenderer, width, height);
    LinkToLinkedListFIFO(&m_pSpriteLinkHead, &m_pSpriteLinkTail, &pSprite->m_LinkInGame);
    return pSprite;
}

void GameManager::DeleteSprite(IGameSprite *pSprite)
{
    Sprite *pS = (Sprite *)pSprite;
    UnLinkFromLinkedList(&m_pSpriteLinkHead, &m_pSpriteLinkTail, &pS->m_LinkInGame);
    delete pS;
}

void GameManager::DeleteAllSprite()
{
    while (m_pSpriteLinkHead)
    {
        Sprite *pSprite = (Sprite *)(m_pSpriteLinkHead->pItem);
        DeleteSprite(pSprite);
    }
}

IGameAnimation *GameManager::CreateAnimationFromFile(const WCHAR *basePath, const WCHAR *filename)
{
    AnimationClip *pClip = nullptr;
    UINT           keySize = wcslen(filename) * sizeof(WCHAR);

    if (m_pAnimationHashTable->Select((void **)&pClip, 1, filename, keySize))
    {
        pClip->AddRef();
    }
    else
    {
        pClip = GeometryGenerator::ReadAnimationFromFile(basePath, filename);

        pClip->m_pSearchHandleInGame = m_pAnimationHashTable->Insert((void *)pClip, filename, keySize);
    }

    return pClip;
}

IGameAnimation *GameManager::CreateEmptyAnimation()
{
    const WCHAR   *filename = L"EmptyAnimation";
    AnimationClip *pClip = nullptr;
    UINT           keySize = wcslen(filename) * sizeof(WCHAR);

    if (m_pAnimationHashTable->Select((void **)&pClip, 1, filename, keySize))
    {
        pClip->AddRef();
    }
    else
    {
        pClip = new AnimationClip;

        pClip->m_pSearchHandleInGame = m_pAnimationHashTable->Insert((void *)pClip, filename, keySize);
    }

    return pClip;
}

IGameAnimation *GameManager::GetAnimationByName(const WCHAR *name)
{
    AnimationClip *pClip = nullptr;
    UINT           keySize = wcslen(name) * sizeof(WCHAR);

    if (m_pAnimationHashTable->Select((void **)&pClip, 1, name, keySize))
    {
        pClip->AddRef();
    }

    return pClip;
}

void GameManager::DeleteAnimation(IGameAnimation *pInAnim)
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

void GameManager::DeleteAllAnimation()
{
    if (m_pAnimationHashTable)
    {
        m_pAnimationHashTable->DeleteAll();
        delete m_pAnimationHashTable;
        m_pAnimationHashTable = nullptr;
    }
}

BOOL GameManager::CreateTerrain(const Material *pMaterial, const int numSlice, const int numStack, const float scale)
{
    if (m_pTerrain)
    {
        m_pTerrain->Release();
        m_pTerrain = nullptr;
    }
    m_pTerrain = m_pRenderer->CreateTerrain(pMaterial, numSlice, numStack, scale);

    if (m_pTerrain)
        return TRUE;
    return FALSE;
}

void GameManager::Register(IController *pController) { m_pControllerManager->Register(pController); }

void GameManager::ToggleCamera() { m_activateCamera = !m_activateCamera; }

GameManager::~GameManager() { Cleanup(); }
