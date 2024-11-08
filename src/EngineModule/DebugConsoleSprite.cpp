#include "pch.h"
#include "DebugConsoleSprite.h"

BOOL DebugConsoleSprite::Initialize(IRenderer *pRnd, UINT width, UINT height, UINT posX, UINT posY, float fontSize)
{
    m_pFontHandle = pRnd->CreateFontObject(L"Tahoma", fontSize);

	m_pSprite = new DynamicSprite;
    m_pSprite->Initialize(pRnd, width, height);
    m_pSprite->SetPosition(posX, posY);

    return TRUE;
}

void DebugConsoleSprite::WriteLine(const WCHAR *line) 
{

}

void DebugConsoleSprite::Update() {}

void DebugConsoleSprite::Render() {}
