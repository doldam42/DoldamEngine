#include "pch.h"

#include <filesystem>

#include "../EngineModule/EngineInterface.h"
#include "../RenderModule/RendererInterface.h"

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

BOOL Client::Initialize(HWND hWnd)
{
    BOOL result = FALSE;
    result = CreateGameEngine(hWnd, &m_pGame);

    m_pRenderer = m_pGame->GetRenderer();
    result = CreateFbxExporter(&m_pFbxExporter);
    result = CreateAssimpExporter(&m_pAssimpExporter);
    m_pFbxExporter->Initialize(m_pGame);
    m_pAssimpExporter->Initialize(m_pGame);

    LoadResources();

    return result;
}

void Client::LoadResources()
{
    fs::path p;

    m_pFontHandle = m_pRenderer->CreateFontObject(L"Tahoma", 18.f);

    /* IGameObject *pBox = m_pGame->CreateGameObject();
     pBox->SetModel(m_pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_BOX));
     pBox->SetScale(2.0f);*/

    // p = L"..\\..\\assets\\stages\\Stage38\\Stage38.dom";
    ////if (!fs::exists(p))
    //{
    //    m_pFbxExporter->Load(L"..\\..\\assets\\stages\\Stage38\\", L"Stage38.fbx");
    //    m_pFbxExporter->ExportModel();
    //}
    // IGameModel  *pStageModel = m_pGame->CreateModelFromFile(L"..\\..\\assets\\stages\\Stage38\\", L"Stage38.dom");
    // IGameObject *pStage = m_pGame->CreateGameObject();
    // pStage->SetModel(pStageModel);
    // pStage->SetScale(10.f);

    p = L"..\\..\\assets\\sponza\\NewSponza_Main_glTF_003.dom";
    if (!fs::exists(p))
    {
        m_pAssimpExporter->Load(L"..\\..\\assets\\sponza\\", L"NewSponza_Main_glTF_003.gltf");
        m_pAssimpExporter->ExportModel();
    }

    IGameModel *pSponzaModel =
        m_pGame->CreateModelFromFile(L"..\\..\\assets\\sponza\\", L"NewSponza_Main_glTF_003.dom");
    IGameObject *pSponza = m_pGame->CreateGameObject();
    pSponza->SetModel(pSponzaModel);
    pSponza->SetScale(30.f);

    /*IGameModel *pGroundModel = m_pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_SQUARE);
    IGameObject *pGround = m_pGame->CreateGameObject();
    pGround->SetModel(pGroundModel);
    pGround->SetRotationX(XM_PIDIV2);
    pGround->SetPosition(0.0f, -0.2f, 0.f);
    pGround->SetScale(2.0f);*/

    p = L"..\\..\\assets\\characters\\gura\\gura.dom";
    if (!fs::exists(p))
    {
        m_pFbxExporter->Load(L"..\\..\\assets\\characters\\gura\\", L"gura.fbx");
        m_pFbxExporter->ExportModel();
    }
    IGameModel     *pGuraModel = m_pGame->CreateModelFromFile(L"..\\..\\assets\\characters\\gura\\", L"gura.dom");
    IGameCharacter *pGura = m_pGame->CreateCharacter();

    pGura->SetModel(pGuraModel);
    pGura->SetRotationX(-XM_PIDIV2);
    p.replace_filename(L"Smolgura_seafoamboy_anims.dca");
    if (!fs::exists(p))
    {
        m_pFbxExporter->LoadAnimation(L"Smolgura_seafoamboy_anims.fbx");
        m_pFbxExporter->ExportAnimation();
    }
    IGameAnimation *pGuraAnim =
        m_pGame->CreateAnimationFromFile(L"..\\..\\assets\\characters\\gura\\", L"Smolgura_seafoamboy_anims.dca");
    pGura->InsertAnimation(pGuraAnim);
    m_pCharacter = pGura;

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
    /* m_pStaticSprite = m_pGame->CreateSpriteFromFile(L"..\\..\\assets\\textures\\", L"wall.jpg", 512, 512);
     m_pStaticSprite->SetPosition(256, 256);
     m_pStaticSprite->SetZ(0.5f);*/
}

void Client::Process()
{
    m_frameCount++;

    ULONGLONG CurTick = GetTickCount64();

    m_pGame->PreUpdate(CurTick);

    Update(CurTick);
    m_pGame->Update(CurTick);

    m_pGame->LateUpdate(CurTick);

    m_pGame->Render();

    if (CurTick - m_prevFrameCheckTick > 1000)
    {
        m_prevFrameCheckTick = CurTick;
        m_FPS = m_frameCount;
        m_frameCount = 0;
    }
}

void Client::Update(ULONGLONG curTick)
{
    // Update Scene with 60FPS
    if (curTick - m_prevUpdateTick < 23)
    {
        return;
    }

    float dt = m_prevUpdateTick == 0 ? 0.0f : static_cast<float>(curTick - m_prevUpdateTick) / 1000.f;
    m_prevUpdateTick = curTick;

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
    Vector3 camPos = m_pGame->GetCameraPos();
    Vector3 camLookAt = m_pGame->GetCameraLookAt();

    int   iTextWidth = 0;
    int   iTextHeight = 0;
    WCHAR wchTxt[260];
    memset(wchTxt, 0, sizeof(wchTxt));
    DWORD dwTxtLen =
        swprintf_s(wchTxt, L"Current FrameRate: %u\nCur Position: (%.3f, %.3f, %.3f)\nLook At: (%.3f, %.3f, %.3f)",
                   m_FPS, camPos.x, camPos.y, camPos.z, camLookAt.x, camLookAt.y, camLookAt.z);

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

    /*float speed = 1.0f;
    Vector3 pos = m_pCharacter->GetPosition();
    m_pCharacter->SetPosition(pos.x, pos.y, pos.z + speed * dt);*/
}

void Client::OnKeyDown(UINT nChar, UINT uiScanCode) { m_pGame->OnKeyDown(nChar, uiScanCode); }

void Client::OnKeyUp(UINT nChar, UINT uiScanCode) { m_pGame->OnKeyUp(nChar, uiScanCode); }

void Client::OnMouseMove(int mouseX, int mouseY) { m_pGame->OnMouseMove(mouseX, mouseY); }

BOOL Client::OnUpdateWindowSize(UINT width, UINT height) { return m_pGame->OnUpdateWindowSize(width, height); }

Client::~Client() { Cleanup(); }
