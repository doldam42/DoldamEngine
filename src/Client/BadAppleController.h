#pragma once

struct VIDEO_HANDLE;
class BadAppleController : public IController
{
  private:
    IGameSprite  *m_pSprite = nullptr;
    IGameObject *m_pSphere = nullptr;
    ITextureHandle *m_pTex = nullptr;
    
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
