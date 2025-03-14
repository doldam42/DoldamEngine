#include "pch.h"

#include "AnimationClip.h"
#include "Camera.h"
#include "Character.h"
#include "ControllerManager.h"
#include "GameObject.h"
#include "GeometryGenerator.h"
#include "MeshObject.h"
#include "Model.h"
#include "PhysicsManager.h"
#include "Sprite.h"
#include "World.h"

#include "GameManager.h"

GameManager *g_pGame = nullptr;

Model *GameManager::SquareMesh = nullptr;
Model *GameManager::BoxMesh = nullptr;
Model *GameManager::SphereMesh = nullptr;

void GameManager::LoadPrimitiveMeshes()
{
    if (!BoxMesh)
    {
        BoxMesh = GeometryGenerator::MakeBox();
        BoxMesh->InitRenderComponents(m_pRenderer);

        size_t id = BoxMesh->GetID();
        BoxMesh->m_pSearchHandleInGame = m_pModelHashTable->Insert((void *)&BoxMesh, (void *)(&id), sizeof(size_t));
    }
    if (!SquareMesh)
    {
        SquareMesh = GeometryGenerator::MakeSquare();
        SquareMesh->InitRenderComponents(m_pRenderer);

        size_t id = SquareMesh->GetID();
        SquareMesh->m_pSearchHandleInGame =
            m_pModelHashTable->Insert((void *)&SquareMesh, (void *)(&id), sizeof(size_t));
    }
    if (!SphereMesh)
    {
        SphereMesh = GeometryGenerator::MakeSphere(1.0f, 32, 32);
        SphereMesh->InitRenderComponents(m_pRenderer);

        size_t id = SphereMesh->GetID();
        SphereMesh->m_pSearchHandleInGame =
            m_pModelHashTable->Insert((void *)&SphereMesh, (void *)(&id), sizeof(size_t));
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
    if (m_pFontHandle)
    {
        m_pRenderer->DeleteFontObject(m_pFontHandle);
        m_pFontHandle = nullptr;
    }
    if (m_pTextImage)
    {
        delete[] m_pTextImage;
        m_pTextImage = nullptr;
    }

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
        m_pRenderer->Release();
        m_pRenderer = nullptr;
    }
    if (m_pMainCamera)
    {
        delete m_pMainCamera;
        m_pMainCamera = nullptr;
    }
}

BOOL GameManager::Initialize(HWND hWnd, IRenderer *pRnd, bool useGUIEditor, UINT viewportWidth, UINT viewportHeight)
{
    BOOL result = FALSE;

    g_pGame = this;

    if (!pRnd)
    {
        __debugbreak();
        return FALSE;
    }

    pRnd->AddRef();
    m_pRenderer = pRnd;

    RECT rect;
    GetClientRect(hWnd, &rect);
    DWORD width = rect.right - rect.left;
    DWORD height = rect.bottom - rect.top;

    m_renderThreadCount = m_pRenderer->GetRenderThreadCount();

    m_pPhysicsManager = new PhysicsManager;
    m_pPhysicsManager->Initialize();

    m_pMainCamera = new Camera;
    if (useGUIEditor)
    {
        m_pMainCamera->Initialize(XMConvertToRadians(90.0f), static_cast<float>(viewportWidth) / viewportHeight, 0.01f,
                                  1000.0f);
        m_useGUIEditor = useGUIEditor;
    }
    else
    {
        m_pMainCamera->Initialize(XMConvertToRadians(90.0f), static_cast<float>(width) / height, 0.01f, 1000.0f);
    }

    m_hWnd = hWnd;

    // Hash Table Initialize
    m_pModelHashTable = new HashTable;
    m_pModelHashTable->Initialize(283, sizeof(size_t), 128);

    m_pGameObjectHashTable = new HashTable;
    m_pGameObjectHashTable->Initialize(283, sizeof(size_t), 128);

    m_pAnimationHashTable = new HashTable();
    m_pAnimationHashTable->Initialize(13, MAX_NAME, 128); // TODO: 최적의 버킷 개수 정하기

    m_pControllerManager = new ControllerManager;

    // m_pInputManager->AddKeyListener(VK_F1, [this](void *) { this->m_isWired = !this->m_isWired; });

    m_pWorld = new World;
    m_pWorld->Initialize();

    m_TextImageWidth = width * 0.4;
    m_TextImageHeight = height * 0.2;

    m_pFontHandle = pRnd->CreateFontObject(L"Tahoma", 21.0f);
    m_pTextSprite = CreateDynamicSprite(m_TextImageWidth, m_TextImageHeight);
    m_pTextSprite->SetPosition(width * 0.6, height * 0.8);
    m_pTextImage = new BYTE[m_TextImageWidth * m_TextImageHeight * 4];
    ZeroMemory(m_pTextImage, sizeof(BYTE) * m_TextImageWidth * m_TextImageHeight * 4);
    ZeroMemory(m_text, sizeof(m_text));

    result = TRUE;
lb_return:
    return result;
}

BOOL GameManager::LoadResources()
{
    // Create Shadow Map Sprite
    // m_pShadowMapSprite = m_pRenderer->CreateSpriteObject();

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
    m_pLight = m_pRenderer->CreateDirectionalLight(&radiance, &direction, &position);

    return TRUE;
}

void GameManager::ProcessInput() {}

void GameManager::Start()
{
    ULONGLONG prevTick = GetTickCount64();

    LoadPrimitiveMeshes();

    LoadResources();

    m_pControllerManager->Start();

    // Update Game Objects
    SORT_LINK *pCur = m_pGameObjLinkHead;
    while (pCur)
    {
        GameObject *pGameObj = (GameObject *)pCur->pItem;

        pGameObj->Update(0.0f);

        pCur = pCur->pNext;
    }
}

void GameManager::Update(float dt)
{
    m_deltaTime = dt;
    PreUpdate(dt);

    // Update Controller
    m_pControllerManager->Update(dt);

    UpdatePhysics(dt);

    // Update Game Objects
    SORT_LINK *pCur = m_pGameObjLinkHead;
    while (pCur)
    {
        GameObject *pGameObj = (GameObject *)pCur->pItem;

        pGameObj->Update(dt);

        if (pGameObj->HasBounds() && m_pMainCamera->IsCulled(pGameObj->GetBounds()))
        {
            pGameObj->m_isVisible = FALSE;
            m_culledObjectCount++;
        }
        else
        {
            pGameObj->m_isVisible = TRUE;
        }

        pCur = pCur->pNext;
    }

    LateUpdate(dt);

    WCHAR text[512] = {L'\0'};
    int   txtLen = wsprintfW(text, L"Culled Object Count :%d", m_culledObjectCount);
    if (wcscmp(text, m_text) != 0)
    {
        // 텍스트가 변경된 경우
        int width, height;
        memset(m_pTextImage, 0, m_TextImageWidth * m_TextImageHeight * 4);
        m_pRenderer->WriteTextToBitmap(m_pTextImage, m_TextImageWidth, m_TextImageHeight, m_TextImageWidth * 4, &width,
                                       &height, m_pFontHandle, text, txtLen);
        m_pTextSprite->UpdateTextureWithImage(m_pTextImage, m_TextImageWidth, m_TextImageHeight);
        wcscpy_s(m_text, text);
    }
    m_culledObjectCount = 0;
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

void GameManager::PreUpdate(float dt)
{
    ProcessInput();
}

void GameManager::UpdatePhysics(float dt)
{
    m_pPhysicsManager->BeginCollision(dt);

    m_pPhysicsManager->ApplyGravityImpulseAll(dt);

    m_pPhysicsManager->CollisionTestAll(m_pWorld, dt);

    m_pPhysicsManager->ResolveContactsAll(dt);
}

void GameManager::LateUpdate(float dt) {}

void GameManager::Render()
{
    // Update Camera
    if (m_pMainCamera->m_isUpdated)
    {
        m_pMainCamera->Update();
        m_pRenderer->UpdateCamera(m_pMainCamera->GetPosition(), m_pMainCamera->GetViewMatrix(),
                                  m_pMainCamera->GetProjMatrix());
    }

    // begin
    m_pRenderer->BeginRender();

    m_pControllerManager->Render();

    if (m_pTerrain)
    {
        m_pRenderer->RenderTerrain(m_pTerrain, &m_terrainScale, m_isWired);
    }

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

    m_pRenderer->EndRender();
    m_pRenderer->Present();
}

BOOL GameManager::OnUpdateWindowSize(UINT width, UINT height, UINT viewportWidth, UINT viewportHeight)
{
    if (m_useGUIEditor)
    {
        m_pMainCamera->SetAspectRatio(float(viewportWidth) / viewportHeight);

        if (m_pRenderer)
        {
            m_pRenderer->OnUpdateWindowSize(width, height, viewportWidth, viewportHeight);
        }
    }
    else
    {
        m_pMainCamera->SetAspectRatio(float(width) / height);

        if (m_pRenderer)
        {
            m_pRenderer->OnUpdateWindowSize(width, height);
        }
    }

    return TRUE;
}

IGameMesh *GameManager::CreateGameMesh()
{
    MeshObject *pObj = new MeshObject;
    return pObj;
}

void GameManager::DeleteGameMesh(IGameMesh *pGameMesh)
{
    MeshObject *pObj = reinterpret_cast<MeshObject *>(pGameMesh);
    delete pObj;
}

IGameCharacter *GameManager::CreateCharacter()
{
    Character *pGameObj = new Character;
    pGameObj->Initialize(this, 5);

    LinkToLinkedListFIFO(&m_pGameObjLinkHead, &m_pGameObjLinkTail, &pGameObj->m_LinkInGame);

    size_t id = pGameObj->GetID();
    pGameObj->m_pSearchHandleInGame = m_pGameObjectHashTable->Insert((void *)pGameObj, (void *)(&id), sizeof(size_t));

    return pGameObj;
}

IGameObject *GameManager::CreateGameObject(BOOL isStatic)
{
    GameObject *pGameObj = new GameObject;
    pGameObj->Initialize(this, isStatic);
    LinkToLinkedListFIFO(&m_pGameObjLinkHead, &m_pGameObjLinkTail, &pGameObj->m_LinkInGame);

    size_t id = pGameObj->GetID();
    pGameObj->m_pSearchHandleInGame = m_pGameObjectHashTable->Insert((void *)pGameObj, (void *)(&id), sizeof(size_t));

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
    if (m_pGameObjectHashTable)
    {
        m_pGameObjectHashTable->DeleteAll();
        delete m_pGameObjectHashTable;
        m_pGameObjectHashTable = nullptr;
    }
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

    size_t id = pModel->GetID();
    pModel->m_pSearchHandleInGame = m_pGameObjectHashTable->Insert((void *)pModel, (void *)(&id), sizeof(size_t));

    return pModel;
}

IGameModel *GameManager::CreateEmptyModel()
{
    Model *pModel = new Model;

    LinkToLinkedListFIFO(&m_pModelLinkHead, &m_pModelLinkTail, &pModel->m_LinkInGame);

    size_t id = pModel->GetID();
    pModel->m_pSearchHandleInGame = m_pGameObjectHashTable->Insert((void *)pModel, (void *)(&id), sizeof(size_t));

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
    if (m_pModelHashTable)
    {
        m_pModelHashTable->DeleteAll();
        delete m_pModelHashTable;
        m_pModelHashTable = nullptr;
    }
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

IGameAnimation *GameManager::CreateEmptyAnimation(const WCHAR *name)
{
    AnimationClip *pClip = new AnimationClip;
    pClip->SetName(name);

    pClip->m_pSearchHandleInGame =
        m_pAnimationHashTable->Insert((void *)pClip, (void *)(pClip->GetName()), sizeof(size_t));

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

BOOL GameManager::CreateTerrain(const Material *pMaterial, const Vector3 *pScale, const int numSlice,
                                const int numStack)
{
    if (m_pTerrain)
    {
        m_pTerrain->Release();
        m_pTerrain = nullptr;
    }
    m_pTerrain = m_pRenderer->CreateTerrain(pMaterial, pScale, numSlice, numStack);
    m_terrainScale = *pScale;

    if (m_pTerrain)
        return TRUE;
    return FALSE;
}

void GameManager::Register(IController *pController) { m_pControllerManager->Register(pController); }

void GameManager::Register(IRenderableController *pController) { m_pControllerManager->Register(pController); }

void GameManager::ToggleCamera() { m_activateCamera = !m_activateCamera; }

BOOL GameManager::Raycast(const Vector3 rayOrigin, const Vector3 rayDir, RayHit *pOutHit, float maxDistance)
{
    Ray ray;
    ray.position = rayOrigin;
    ray.direction = rayDir;
    ray.tmax = maxDistance;

    //m_pWorld->Intersect(ray, pOutHit)
    return m_pPhysicsManager->IntersectRay(ray, pOutHit);
    //RayHit rayHit;
    //float  closestHit = m_pWorld->Intersect(ray, &rayHit) ? rayHit.tHit : FLT_MAX;

    //SORT_LINK *pCur = m_pGameObjLinkHead;
    //while (pCur)
    //{
    //    GameObject *pObj = (GameObject *)pCur->pItem;

    //    float hitt0, hitt1;
    //    if (pObj->IntersectRay(ray, &hitt0, &hitt1) && hitt0 < closestHit)
    //    {
    //        closestHit = hitt0;
    //        rayHit.pHitted = pObj;
    //        rayHit.tHit = hitt0;
    //    }

    //    pCur->pNext;
    //}

    //if (closestHit < FLT_MAX)
    //{
    //    *pOutHit = rayHit;
    //    return TRUE;
    //}

    //return FALSE;
}

GameManager::~GameManager()
{
    Cleanup();
#ifdef _DEBUG
    _ASSERT(_CrtCheckMemory());
#endif
}

HRESULT __stdcall GameManager::QueryInterface(REFIID riid, void **ppvObject) { return E_NOTIMPL; }

ULONG __stdcall GameManager::AddRef(void)
{
    m_refCount++;
    return m_refCount;
}

ULONG __stdcall GameManager::Release(void)
{
    ULONG ref_count = --m_refCount;
    if (!m_refCount)
        delete this;

    return ref_count;
}
