#pragma once

#include <fmod/fmod.hpp>
#include <fmod/fmod_errors.h>

struct AUDIO_HANDLE
{
    FMOD::Sound   *pSound = nullptr;
    FMOD::Channel *pChannel = nullptr;
    bool           isPlay = false;
};

class AudioManager : public IController
{
  private:
    FMOD::System *m_pSystem = nullptr;

  private:
    void Cleanup();

  public:
    void Initialize();
    BOOL Start() override;
    void Update(float dt) override;
    
    AUDIO_HANDLE *CreateAudioHandle(const WCHAR *wpath);
    void          DeleteAudioHandle(AUDIO_HANDLE *pDel);
    void          SoundPlay(AUDIO_HANDLE *pAudio, bool isLoop);

    AudioManager() = default;
    ~AudioManager();
};
