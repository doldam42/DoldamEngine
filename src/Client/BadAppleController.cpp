#include "pch.h"

#include "Client.h"
#include "VideoManager.h"

#include "BadAppleController.h"

void BadAppleController::Cleanup()
{
    IGameManager *pGame = g_pClient->GetGameManager();
    if (m_pBadAppleVideo)
    {
        DeleteVideoHandle(m_pBadAppleVideo);
        m_pBadAppleVideo = nullptr;
    }
    /*if (m_pSprite)
    {
        pGame->DeleteSprite(m_pSprite);
        m_pSprite = nullptr;
    }*/
}

BOOL BadAppleController::Start()
{
    BOOL result = TRUE;

    IGameManager *pGame = g_pClient->GetGameManager();
    IRenderer    *pRnd = pGame->GetRenderer();
    result = CreateVideoHandle(&m_pBadAppleVideo, L"bad_apple.mp4");

    // Create Textures
    m_pTex = pRnd->CreateDynamicTexture(2 * m_pBadAppleVideo->width, m_pBadAppleVideo->height);

    IRenderMaterial *pDynamicMaterial = pRnd->CreateDynamicMaterial(L"bad_apple");
    pDynamicMaterial->UpdateTextureWithTexture(m_pTex, TEXTURE_TYPE_ALBEDO);
    pDynamicMaterial->UpdateMetallicRoughness(0.8f, 0.8f);

    m_pSprite = pGame->CreateDynamicSprite(m_pBadAppleVideo->width, m_pBadAppleVideo->height);
    m_pSprite->SetPosition(0, 0);

    IGameModel *pModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_SPHERE);

    m_pSphere = pGame->CreateGameObject();
    m_pSphere->SetModel(pModel);
    m_pSphere->SetPosition(5.0f, 5.05f, 5.0f);
    m_pSphere->SetScale(4.0f);
    m_pSphere->SetMaterials(&pDynamicMaterial, 1);
    /*
    Sphere sphere(2.0f);
    m_pSphere->InitPhysics(&sphere, 0.0f, 0.8f, 0.5f);*/

    return result;
}

void BadAppleController::Update(float dt) 
{ 
    IRenderer *pRnd = g_pClient->GetGameManager()->GetRenderer();
    VideoPlay(m_pBadAppleVideo, dt);
    m_pSprite->UpdateTextureWidthImage(m_pBadAppleVideo->pRGBAImage, m_pBadAppleVideo->width,
                                       m_pBadAppleVideo->height);

    pRnd->UpdateTextureWithImage(m_pTex, m_pBadAppleVideo->pRGBAImage, m_pBadAppleVideo->width,
                                 m_pBadAppleVideo->height);
}

BadAppleController::~BadAppleController() { Cleanup(); }
