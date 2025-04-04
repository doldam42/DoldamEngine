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

SOUND_HANDLE *AudioManager::CreateAudioHandle(const WCHAR *wpath)
{
    char path[MAX_PATH] = {'\0'};
    ws2s(wpath, path);

    SOUND_HANDLE *pNew = new SOUND_HANDLE;
    if (m_pSystem->createSound(path, FMOD_DEFAULT, 0, &pNew->pSound) != FMOD_OK)
    {
        return nullptr;
    }
    
    return pNew;
}

void AudioManager::DeleteAudioHandle(SOUND_HANDLE *pDel)
{
    if (pDel->pSound)
    {
        pDel->pSound->release();
        pDel->pSound = nullptr;
    }
    delete pDel;
}

void AudioManager::SoundPlay(SOUND_HANDLE *pAudio, bool isLoop)
{
    if (pAudio->pChannel)
    {
        pAudio->pChannel->isPlaying(&pAudio->isPlay);
    }
    if (!pAudio->isPlay)
    {
        FMOD_RESULT ret = m_pSystem->playSound(pAudio->pSound, nullptr, pAudio->isPaused, &pAudio->pChannel);
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
