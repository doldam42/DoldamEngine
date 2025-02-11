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
    void Initialize();
    BOOL Start() override;
    void Update(float dt) override;

    SOUND_HANDLE *CreateAudioHandle(const WCHAR *wpath);
    void          DeleteAudioHandle(SOUND_HANDLE *pDel);
    void          SoundPlay(SOUND_HANDLE *pAudio, bool isLoop);

    AudioManager() = default;
    ~AudioManager();

    // Inherited via IController
    void Render() override;
};
