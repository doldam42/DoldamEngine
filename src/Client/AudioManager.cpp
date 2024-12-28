#include "pch.h"
#include "AudioManager.h"

void AudioManager::Cleanup()
{
    if (m_pSystem)
    {
        m_pSystem->close();
        m_pSystem->release();
        m_pSystem = nullptr;
    }
}

void AudioManager::Initialize()
{
    FMOD::System_Create(&m_pSystem);
    m_pSystem->init(4, FMOD_INIT_NORMAL, NULL);
}

AUDIO_HANDLE *AudioManager::CreateAudioHandle(const WCHAR *wpath)
{
    char path[MAX_PATH] = {'\0'};
    ws2s(wpath, path);

    AUDIO_HANDLE *pNew = new AUDIO_HANDLE;
    if (m_pSystem->createSound(path, FMOD_DEFAULT, 0, &pNew->pSound) != FMOD_OK)
    {
        return nullptr;
    }
    
    return pNew;
}

void AudioManager::DeleteAudioHandle(AUDIO_HANDLE *pDel)
{
    if (pDel->pSound)
    {
        pDel->pSound->release();
        pDel->pSound = nullptr;
    }
    delete pDel;
}

void AudioManager::SoundPlay(AUDIO_HANDLE *pAudio, bool isLoop)
{
    if (pAudio->pChannel)
    {
        pAudio->pChannel->isPlaying(&pAudio->isPlay);
    }
    if (!pAudio->isPlay)
    {
        FMOD_RESULT ret = m_pSystem->playSound(pAudio->pSound, nullptr, false, &pAudio->pChannel);
        if (ret == FMOD_OK)
        {
            if (isLoop)
            {
                pAudio->pChannel->setMode(FMOD_LOOP_NORMAL);
            }
            else
            {
                pAudio->pChannel->setMode(FMOD_LOOP_OFF);
            }
        }
    }
}

AudioManager::~AudioManager() { Cleanup(); }

BOOL AudioManager::Start() { return 0; }

void AudioManager::Update(float dt) { m_pSystem->update(); }
