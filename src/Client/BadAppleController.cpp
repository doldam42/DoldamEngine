#include "pch.h"

#include "Client.h"

#include "AudioManager.h"
#include "VideoManager.h"

#include "InputManager.h"

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

    if (m_ppSpheres)
    {
        for (UINT i = 0; i < m_numSpheres; i++)
        {
            pGame->DeleteGameObject(m_ppSpheres[i]);
            m_ppSpheres[i] = nullptr;
        }
        delete[] m_ppSpheres;
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

    result = CreateVideoHandle(&m_pBadAppleVideo, L"new_bad_apple.mp4");

    SOUND_HANDLE *pBadAppleAudio = pAudio->CreateAudioHandle(L"new_bad_apple.mp3");
    if (!pBadAppleAudio)
    {
        __debugbreak();
    }
    
    m_pBadAppleAudio = pBadAppleAudio;
    
    // Create Textures
    m_pTex = pRnd->CreateDynamicTexture(m_pBadAppleVideo->width, m_pBadAppleVideo->height);

    IRenderMaterial *pDynamicMaterial = pRnd->CreateDynamicMaterial(L"bad_apple");
    pDynamicMaterial->UpdateTextureWithTexture(m_pTex, TEXTURE_TYPE_ALBEDO);
    pDynamicMaterial->UpdateMetallicRoughness(0.8f, 0.8f);

    m_pSprite = pGame->CreateDynamicSprite(m_pBadAppleVideo->width, m_pBadAppleVideo->height);
    m_pSprite->SetPosition(0, 0);
    m_pSprite->SetScale(0.7f);

    IGameModel *pModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_SPHERE);

    UINT numRows = m_pBadAppleVideo->height / 120;
    UINT numCols = m_pBadAppleVideo->width / 120;

    m_numSpheres = (numRows + 2) * (numCols + 1);
    m_ppSpheres = new IGameObject *[m_numSpheres];
    memset(m_ppSpheres, 0, sizeof(IGameObject *) * m_numSpheres);
    
    // width(480), height(360)
    // numCols(4), numRows(3)
    // [-16, 0, 2] ~ [16, 32, 2]
    UINT offset = 8;

    float x = -16.0f;
    float y = 1e-3f;
    
    int i = 0;
    for (UINT row = 0; row < numRows + 2; row++)
    {
        for (UINT col = 0; col < numCols + 1; col++)
        {
            IGameObject* pObj = pGame->CreateGameObject(TRUE);
            pModel->AddRef();
            pObj->SetModel(pModel);
            pObj->SetMaterials(&pDynamicMaterial, 1);
            pObj->SetPosition(x + col * offset, y + row * offset, 20);
            pObj->SetScale(3.9f);
            m_ppSpheres[i] = pObj;
            i++;
        }
    }

    float  aspect = m_pBadAppleVideo->width / m_pBadAppleVideo->height;
    Camera projectorCam;
    projectorCam.Initialize(XMConvertToRadians(90.0f), aspect, 0.1f, 10.f);
    projectorCam.SetEyeAtUp(Vector3(0.f, 16.f, -2.f), Vector3(0.0f, 16.f, 1.0f), Vector3(0.f, 1.0f, 0.0f));
    projectorCam.Update();

    pRnd->SetProjectionTexture(m_pTex);
    pRnd->SetProjectionTextureViewProj(&projectorCam.GetViewMatrix(), &projectorCam.GetProjMatrix());

    pAudio->SoundPlay(m_pBadAppleAudio, true);

    return result;
}

void BadAppleController::Update(float dt)
{
    static bool   isPaused = false;
    InputManager *pI = g_pClient->GetInputManager();
    if (pI->IsKeyPressed(VK_SPACE, false))
    {
        isPaused = !isPaused;
        m_pBadAppleAudio->isPaused = isPaused;
        m_pBadAppleAudio->pChannel->setPaused(isPaused);
    }

    if (isPaused)
        return;

    IRenderer    *pRnd = g_pClient->GetGameManager()->GetRenderer();
    
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

