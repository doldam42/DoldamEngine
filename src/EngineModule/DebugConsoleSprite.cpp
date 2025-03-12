#include "pch.h"
#include "DebugConsoleSprite.h"

BOOL DebugConsoleSprite::Initialize(IRenderer *pRnd, UINT width, UINT height, UINT posX, UINT posY, float fontSize)
{
    m_pFontHandle = pRnd->CreateFontObject(L"Tahoma", fontSize);

    m_pSprite = new DynamicSprite;
    m_pSprite->Initialize(pRnd, width, height);
    m_pSprite->SetPosition(posX, posY);

    m_pImage = new BYTE[width * height * 4];

    ZeroMemory(m_pImage, sizeof(BYTE) * width * height * 4);
    ZeroMemory(m_text, sizeof(m_text));
    ZeroMemory(m_prevText, sizeof(m_prevText));

    m_width = width;
    m_height = height;

    m_pRenderer = pRnd;

    return TRUE;
}

void DebugConsoleSprite::WriteLine(const WCHAR *line) { wcscat_s(m_text, line); }

void DebugConsoleSprite::Update() 
{ 
    if (wcscmp(m_text, m_prevText) != 0)
    {
        int width, height;
        m_pRenderer->WriteTextToBitmap(m_pImage, m_width, m_height, m_width * 4, &width, &height, m_pFontHandle, m_text,
                                       wcslen(m_text));

        m_pSprite->UpdateTextureWithImage(m_pImage, width, height);
        
        ZeroMemory(m_prevText, sizeof(m_prevText));
        wcscpy_s(m_prevText, m_text);
    }

    ZeroMemory(m_text, sizeof(m_text));
}

void DebugConsoleSprite::Render() { m_pSprite->Render(); }
