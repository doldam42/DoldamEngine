#pragma once

#include "Sprite.h"

class DebugConsoleSprite
{
    IRenderer     *m_pRenderer = nullptr;
    DynamicSprite *m_pSprite = nullptr;
    IFontHandle   *m_pFontHandle = nullptr;
    WCHAR          m_text[1024];

  public:
    BOOL Initialize(IRenderer *pRnd, UINT width, UINT height, UINT posX, UINT posY, float fontSize);

    void WriteLine(const WCHAR *line);

    void Update();
    void Render();
};
