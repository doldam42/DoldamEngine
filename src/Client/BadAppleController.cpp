#include "pch.h"

#include "Client.h"

#include "AudioManager.h"
#include "VideoManager.h"

#include "BadAppleController.h"

void BadAppleController::Cleanup()
{
    AudioManager *pAudio = g_pClient->GetAudioManager();
    IGameManager *pGame = g_pClient->GetGameManager();
    IRenderer    *pRnd = pGame->GetRenderer();
    if (m_pBadAppleVideo)
    {
        DeleteVideoHandle(m_pBadAppleVideo);
        m_pBadAppleVideo = nullptr;
    }
    if (m_pBadAppleAudio)
    {
        pAudio->DeleteAudioHandle(m_pBadAppleAudio);
        m_pBadAppleAudio = nullptr;
    }
    if (m_pSprite)
    {
        pGame->DeleteSprite(m_pSprite);
        m_pSprite = nullptr;
    }
    if (m_pSphere)
    {
        pGame->DeleteGameObject(m_pSphere);
        m_pSphere = nullptr;
    }
    if (m_pTex)
    {
        pRnd->DeleteTexture(m_pTex);
        m_pTex = nullptr;
    }
}

BOOL BadAppleController::Start()
{
    BOOL result = TRUE;

    AudioManager *pAudio = g_pClient->GetAudioManager();
    IGameManager *pGame = g_pClient->GetGameManager();
    IRenderer    *pRnd = pGame->GetRenderer();
    
    result = CreateVideoHandle(&m_pBadAppleVideo, L"bad_apple.mp4");
    
    m_pBadAppleAudio = pAudio->CreateAudioHandle(L"bad_apple.mp3");
    if (!m_pBadAppleAudio)
    {
        __debugbreak();
    }

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
    AudioManager *pAudio = g_pClient->GetAudioManager();
    IRenderer *pRnd = g_pClient->GetGameManager()->GetRenderer();

    pAudio->SoundPlay(m_pBadAppleAudio, true);
    VideoPlay(m_pBadAppleVideo, dt);

    if (m_pBadAppleVideo->isUpdated)
    {
        m_pSprite->UpdateTextureWithImage(m_pBadAppleVideo->pRGBAImage, m_pBadAppleVideo->width,
                                           m_pBadAppleVideo->height);

        pRnd->UpdateTextureWithImage(m_pTex, m_pBadAppleVideo->pRGBAImage, m_pBadAppleVideo->width,
                                     m_pBadAppleVideo->height);

        m_pBadAppleVideo->isUpdated = FALSE;
    }
}

BadAppleController::~BadAppleController() { Cleanup(); }
