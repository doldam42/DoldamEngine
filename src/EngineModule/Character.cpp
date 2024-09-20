#include "pch.h"
#include "GameEngine.h"
#include "Model.h"
#include "Character.h"

void Character::LateUpdate(float dt)
{
    static int     frameCount = 0;
    AnimationClip &clip = g_pGame->GetAnimationByName(L"gura.dca");
    GetModel()->UpdateAnimation(&clip, frameCount);
    frameCount++;
    frameCount %= 3600;
}
