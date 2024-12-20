#include "pch.h"

#include <filesystem>

#include "../EngineModule/EngineInterface.h"
#include "../RenderModule/RendererInterface.h"

#include "TimeController.h"

#include "Client.h"

namespace fs = std::filesystem;

void Client::Cleanup()
{
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
    if (m_pImage)
    {
        delete[] m_pImage;
        m_pImage = nullptr;
    }
    if (m_pStaticSprite)
    {
        m_pGame->DeleteSprite(m_pStaticSprite);
        m_pStaticSprite = nullptr;
    }
    if (m_pDynamicSprite)
    {
        m_pGame->DeleteSprite(m_pDynamicSprite);
        m_pDynamicSprite = nullptr;
    }
    if (m_pTextSprite)
    {
        m_pGame->DeleteSprite(m_pTextSprite);
        m_pTextSprite = nullptr;
    }
    if (m_pGame)
    {
        DeleteGameEngine(m_pGame);
        m_pGame = nullptr;
    }
}

Client &Client::GetInstance()
{ 
    static Client instance;
    return instance;
}

BOOL Client::Initialize(HWND hWnd)
{
    BOOL result = FALSE;
    result = CreateGameEngine(hWnd, &m_pGame);

    m_pRenderer = m_pGame->GetRenderer();
    result = CreateFbxExporter(&m_pFbxExporter);
    result = CreateAssimpExporter(&m_pAssimpExporter);
    m_pFbxExporter->Initialize(m_pGame);
    m_pAssimpExporter->Initialize(m_pGame);

    // Register Controllers Before Start Game Manager.
    m_pGame->Register(this);
    m_pGame->Register(&m_timeController);

    m_pGame->Start();

    return result;
}

void Client::LoadResources()
{
    fs::path p;

    m_pFontHandle = m_pRenderer->CreateFontObject(L"Tahoma", 18.f);

    // Create Materials
    Material mat;
    wcscpy_s(mat.name, L"wall");
    wcscpy_s(mat.basePath, L"..\\..\\assets\\textures\\");
    wcscpy_s(mat.albedoTextureName, L"wall.jpg");
    mat.roughnessFactor = 0.2f;
    mat.metallicFactor = 0.8f;
    mat.opacityFactor = 1.0f;
    IMaterialHandle *pWallMaterial = m_pRenderer->CreateMaterialHandle(&mat);

    mat = Material();
    mat.roughnessFactor = 0.1f;
    mat.metallicFactor = 0.8f;
    wcscpy_s(mat.name, L"ground");
    wcscpy_s(mat.basePath, L"..\\..\\assets\\textures\\");
    wcscpy_s(mat.albedoTextureName, L"blender_uv_grid_2k.png");
    IMaterialHandle *pGroundMaterial = m_pRenderer->CreateMaterialHandle(&mat);

    IGameModel *pModel = m_pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_SPHERE);
    IGameMesh  *pMesh = pModel->GetMeshAt(0);
    pMesh->UpdateMaterial(pWallMaterial, 0);

    IGameObject *pSphere1 = m_pGame->CreateGameObject();
    IGameObject *pSphere2 = m_pGame->CreateGameObject();

    pSphere1->SetModel(pModel);
    pSphere1->SetPosition(0.0f, 5.0f, 3.0f);

    pModel->AddRef();
    pSphere2->SetModel(pModel);
    pSphere2->SetPosition(0.0f, 6.0f, 3.0f);

    Sphere sphere(1.0f);
    pSphere1->InitPhysics(&sphere, 1.0f, 0.8f, 0.5f);
    pSphere2->InitPhysics(&sphere, 1.0f, 0.8f, 0.5f);

    m_pSphere = pSphere1;

    IGameModel *pGroundModel = m_pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_SPHERE);
    IGameObject *pGround = m_pGame->CreateGameObject();
    pGround->SetModel(pGroundModel);
    pGround->SetPosition(0.0f, -102.f, 0.f);
    pGround->SetScale(100.0f);

    sphere.Radius = 100.0f;
    pGround->InitPhysics(&sphere, 0.0f, 1.0f, 0.5f);
    pGroundModel->GetMeshAt(0)->UpdateMaterial(pGroundMaterial, 0);

    // Create texture from draw Text
    m_textImageWidth = 712;
    m_textImageHeight = 256;
    m_pTextImage = new BYTE[m_textImageWidth * m_textImageHeight * 4];

    // Create Image
    UINT   imageWidth = 512;
    UINT   imageHeight = 256;
    BYTE  *pImage = new BYTE[imageWidth * imageHeight * 4];
    DWORD *pDest = (DWORD *)pImage;
    for (DWORD y = 0; y < imageHeight; y++)
    {
        for (DWORD x = 0; x < imageWidth; x++)
        {
            pDest[x + imageWidth * y] = 0xff0000ff;
        }
    }
    m_pImage = pImage;

    m_pTextSprite = m_pGame->CreateDynamicSprite(m_textImageWidth, m_textImageHeight);
    m_pTextSprite->SetPosition(512 + 5, 256 + 5 + 256 + 5);
    m_pDynamicSprite = m_pGame->CreateDynamicSprite(imageWidth, imageHeight);
    m_pDynamicSprite->SetPosition(0, 512);
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

void Client::Update(float dt)
{
    // Update Texture
    static DWORD g_dwCount = 0;
    static DWORD g_dwTileColorR = 0;
    static DWORD g_dwTileColorG = 0;
    static DWORD g_dwTileColorB = 0;

    const DWORD TILE_WIDTH = 16;
    const DWORD TILE_HEIGHT = 16;

    DWORD TILE_WIDTH_COUNT = 512 / TILE_WIDTH;
    DWORD TILE_HEIGHT_COUNT = 256 / TILE_HEIGHT;

    if (g_dwCount >= TILE_WIDTH_COUNT * TILE_HEIGHT_COUNT)
    {
        g_dwCount = 0;
    }
    DWORD TileY = g_dwCount / TILE_WIDTH_COUNT;
    DWORD TileX = g_dwCount % TILE_WIDTH_COUNT;

    DWORD StartX = TileX * TILE_WIDTH;
    DWORD StartY = TileY * TILE_HEIGHT;

    // DWORD r = rand() % 256;
    // DWORD g = rand() % 256;
    // DWORD b = rand() % 256;

    DWORD r = g_dwTileColorR;
    DWORD g = g_dwTileColorG;
    DWORD b = g_dwTileColorB;

    DWORD *pDest = (DWORD *)m_pImage;
    for (DWORD y = 0; y < 16; y++)
    {
        for (DWORD x = 0; x < 16; x++)
        {
            if (StartX + x >= 512)
                __debugbreak();

            if (StartY + y >= 256)
                __debugbreak();

            pDest[(StartX + x) + (StartY + y) * 512] = 0xff000000 | (b << 16) | (g << 8) | r;
        }
    }
    g_dwCount++;
    g_dwTileColorR += 8;
    if (g_dwTileColorR > 255)
    {
        g_dwTileColorR = 0;
        g_dwTileColorG += 8;
    }
    if (g_dwTileColorG > 255)
    {
        g_dwTileColorG = 0;
        g_dwTileColorB += 8;
    }
    if (g_dwTileColorB > 255)
    {
        g_dwTileColorB = 0;
    }

    m_pDynamicSprite->UpdateTextureWidthImage(m_pImage, 512, 256);

    // draw text
    UINT    fps = m_pGame->FPS();
    Vector3 pos = m_pSphere->GetPosition();

    float rotX = m_pSphere->GetRotationX();
    float rotY = m_pSphere->GetRotationY();
    float rotZ = m_pSphere->GetRotationZ();

    int   iTextWidth = 0;
    int   iTextHeight = 0;
    WCHAR wchTxt[260];
    memset(wchTxt, 0, sizeof(wchTxt));
    DWORD dwTxtLen =
        swprintf_s(wchTxt, L"Current FrameRate: %u\nCur Position: (%.3f, %.3f, %.3f)\nOrient: (%.3f, %.3f, %.3f)", fps,
                   pos.x, pos.y, pos.z, rotX, rotY, rotZ);

    if (wcscmp(m_text, wchTxt))
    {
        // 텍스트가 변경된 경우
        m_pRenderer->WriteTextToBitmap(m_pTextImage, m_textImageWidth, m_textImageHeight, m_textImageWidth * 4,
                                       &iTextWidth, &iTextHeight, m_pFontHandle, wchTxt, dwTxtLen);
        m_pTextSprite->UpdateTextureWidthImage(m_pTextImage, m_textImageWidth, m_textImageHeight);
        wcscpy_s(m_text, wchTxt);
    }
    else
    {
        // 텍스트가 변경되지 않은 경우 - 업데이트 할 필요 없다.
        int a = 0;
    }
}

void Client::OnKeyDown(UINT nChar, UINT uiScanCode) { m_pGame->OnKeyDown(nChar, uiScanCode); }

void Client::OnKeyUp(UINT nChar, UINT uiScanCode) { m_pGame->OnKeyUp(nChar, uiScanCode); }

void Client::OnMouseMove(int mouseX, int mouseY) { m_pGame->OnMouseMove(mouseX, mouseY); }

void Client::OnMouseWheel(float deltaWheel) { m_pGame->OnMouseWheel(deltaWheel); }

BOOL Client::OnUpdateWindowSize(UINT width, UINT height) { return m_pGame->OnUpdateWindowSize(width, height); }

Client::~Client() { Cleanup(); }
