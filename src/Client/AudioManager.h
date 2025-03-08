#pragma once

#include <fmod/fmod.hpp>
#include <fmod/fmod_errors.h>

struct SOUND_HANDLE
{
    FMOD::Sound   *pSound = nullptr;
    FMOD::Channel *pChannel = nullptr;

    bool isPlay = false;
    bool isPaused = false;
};

class AudioManager : public IController
{
  private:
    FMOD::System *m_pSystem = nullptr;

  private:
    void Cleanup();

  public:
    BOOL Start() override { return TRUE; }
    void Update(float dt) override { m_pSystem->update(); }

    SOUND_HANDLE *CreateAudioHandle(const WCHAR *wpath);
    void          DeleteAudioHandle(SOUND_HANDLE *pDel);
    void          SoundPlay(SOUND_HANDLE *pAudio, bool isLoop);

    AudioManager() {
        FMOD::System_Create(&m_pSystem);
        m_pSystem->init(4, FMOD_INIT_NORMAL, NULL);
    }
    ~AudioManager();
};
