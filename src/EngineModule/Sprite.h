#pragma once

class GameEngine;
class Sprite : public IGameSprite
{
  protected:
    IRenderSprite *m_pSprite = nullptr;
    IRenderer     *m_pRenderer = nullptr;

    UINT m_width;
    UINT m_height;

    int m_posX = 0;
    int m_posY = 0;

    float m_Z = 0.0f;
    float m_scaleX = 1.0f;
    float m_scaleY = 1.0f;

  public:
    SORT_LINK m_LinkInGame;

  private:
    void Cleanup();

  public:
    BOOL Initialize(IRenderer *pRnd, const WCHAR *filename, UINT width, UINT height);

    virtual void Render();

    BOOL UpdateTextureWidthImage(const BYTE *pSrcBits, UINT srcWidth, UINT srcHeight) override;

    // Getter
    UINT GetWidth() override { return m_width; }
    UINT GetHeight() override { return m_height; }

    int GetPosX() override { return m_posX; }
    int GetPosY() override { return m_posY; }

    float GetScaleX() override { return m_scaleX; }
    float GetScaleY() override { return m_scaleY; }

    float GetZ() override { return m_Z; }

    // Setter
    void SetPosition(int posX, int posY) override
    {
        m_posX = posX;
        m_posY = posY;
    }
    void SetScale(float scaleX, float scaleY) override
    {
        m_scaleX = scaleX;
        m_scaleY = scaleY;
    }
    void SetSize(UINT width, UINT height) override
    {
        m_width = width;
        m_height = height;
    };
    void SetScale(float scale) override { m_scaleX = m_scaleY = scale; }
    void SetZ(float z) override { m_Z = z; }

    Sprite() = default;
    virtual ~Sprite();
};

class DynamicSprite : public Sprite
{
    ITextureHandle *m_pTexHandle = nullptr;

  private:
    void Cleanup();

  public:
    BOOL Initialize(IRenderer *pRnd, UINT width, UINT height);
    virtual void Render() override;

    BOOL UpdateTextureWidthImage(const BYTE *pSrcBits, UINT srcWidth, UINT srcHeight) override;

    DynamicSprite() = default;
    ~DynamicSprite();
};
