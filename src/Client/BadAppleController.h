#pragma once

struct VIDEO_HANDLE;
class BadAppleController : public IController
{
  private:
    IGameSprite  *m_pSprite = nullptr;
    IGameObject  **m_ppSpheres = nullptr;
    ITextureHandle *m_pTex = nullptr;
    
    UINT m_numSpheres = 0;

    SOUND_HANDLE *m_pBadAppleAudio = nullptr;
    VIDEO_HANDLE *m_pBadAppleVideo = nullptr;

  private:
    void Cleanup();

  public:
    // Inherited via IController
    BOOL Start() override;
    void Update(float dt) override;

    BadAppleController() = default;
    ~BadAppleController();
};
