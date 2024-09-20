#include "pch.h"

#include "Sprite.h"

void Sprite::Cleanup() 
{
    if (m_pSprite)
    {
        m_pSprite->Release();
        m_pSprite = nullptr;
    }
}

BOOL Sprite::Initialize(IRenderer *pRnd, const WCHAR *filename, UINT width, UINT height)
{
    IRenderSprite *pSprite = pRnd->CreateSpriteObject(filename, 0, 0, width, height);

    m_width = width;
    m_height = height;

    if (!pSprite)
        return FALSE;

    m_pRenderer = pRnd;
    m_pSprite = pSprite;

    m_LinkInGame.pPrev = nullptr;
    m_LinkInGame.pNext = nullptr;
    m_LinkInGame.pItem = this;

    return TRUE;
}

void Sprite::Render() 
{ 
    m_pRenderer->RenderSprite(m_pSprite, m_posX, m_posY, m_scaleX, m_scaleY, m_Z); 
}

BOOL Sprite::UpdateTextureWidthImage(const BYTE *pSrcBits, UINT srcWidth, UINT srcHeight) { return FALSE; }

Sprite::~Sprite() { Cleanup(); }

void DynamicSprite::Cleanup()
{
    if (m_pTexHandle)
    {
        m_pRenderer->DeleteTexture(m_pTexHandle);
        m_pTexHandle = nullptr;
    }
}

BOOL DynamicSprite::Initialize(IRenderer *pRnd, UINT width, UINT height)
{
    IRenderSprite  *pSprite = pRnd->CreateSpriteObject();
    ITextureHandle *pTexHandle = pRnd->CreateDynamicTexture(width, height);

    if (!pSprite || !pTexHandle)
        return FALSE;

    m_pSprite = pSprite;
    m_pTexHandle = pTexHandle;
    m_pRenderer = pRnd;

    m_LinkInGame.pPrev = nullptr;
    m_LinkInGame.pNext = nullptr;
    m_LinkInGame.pItem = this;

    return TRUE;
}

void DynamicSprite::Render()
{
    m_pRenderer->RenderSpriteWithTex(m_pSprite, m_posX, m_posY, m_scaleX, m_scaleY, nullptr, m_Z, m_pTexHandle);
}

BOOL DynamicSprite::UpdateTextureWidthImage(const BYTE *pSrcBits, UINT srcWidth,
                                            UINT srcHeight)
{
    UINT width = (srcWidth < m_width) ? srcWidth : m_width;
    UINT height = (srcHeight < m_height) ? srcHeight : m_height;

    m_pRenderer->UpdateTextureWithImage(m_pTexHandle, pSrcBits, width, height);

    return TRUE;
}

DynamicSprite::~DynamicSprite() { Cleanup(); }
